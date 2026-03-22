#include "RequestLineValidator.hpp"

RequestLineValidator::RequestLineValidator( ParseContext &parse_context ) :
	_parse_context(parse_context) { }

bool RequestLineValidator::_addValueToMap(
	std::regex reg_method,
	std::string &buffer,
	const char *errmsg,
	std::string key
)
{
	std::string value;
	if (!RequestStringUtils::tryExtractHeaderField(value, buffer, reg_method, errmsg)) {
		return false;
	}
	_parse_context.request.set_header_value(key, value);
	return true;
}

bool RequestLineValidator::_isValidRequestLine(std::string &buffer)
{
	if (buffer.empty() || buffer.length() > http::limits::max_header_value_length) return false;

	bool result = _addValueToMap(HttpRegexPatterns::METHOD(), buffer, ERROR_HTTP_METHOD, http::headers::METHOD)
	&& _addValueToMap(HttpRegexPatterns::FILEPATH(), buffer, ERROR_HTTP_REQUEST_TARGET, http::headers::REQUEST_TARGET);
	_parse_context.request.set_header_value("version", buffer);
	return result;
}

bool RequestLineValidator::_isValidHttpVersion()
{
	if (_parse_context.request.get_header_value("version") != "HTTP/1.1") {
		_parse_context.request.set_status_code(HttpStatus::e_code::HTTP_VERSION_NOT_SUPPORTED);
		std::cerr << _parse_context.request.get_status_code() << std::endl;
		return false;
	}
	return true;
}

bool RequestLineValidator::_isValidUriLength()
{
	if (_parse_context.request.get_header_value(http::headers::REQUEST_TARGET_DECODED).length() > http::limits::max_uri_length) {
		_parse_context.request.set_status_code(HttpStatus::e_code::URI_TOO_LONG);
		std::cerr << _parse_context.request.get_status_code() << std::endl;
		return false;
	}
	return true;
}

bool RequestLineValidator::_isMethodAllowed()
{
	auto mr = _parse_context.request.getFile().getMethodRegistry();
	if (!mr.isAllowed(_parse_context.request.get_header_value(http::headers::METHOD))) {
		_parse_context.request.set_status_code(HttpStatus::e_code::METHOD_NOT_ALLOWED);
		std::cerr << _parse_context.request.get_status_code() << std::endl;
		return false;
	}
	return true;
}

bool RequestLineValidator::_isCGIPathValid()
{
	std::string	target = _parse_context.request.get_header_value(http::headers::REQUEST_TARGET);

	std::string path = RegexMatcher::get_regex_value(target, HttpRegexPatterns::CGI_VALID_PATH());

	if (path.empty())
		return false;

	return true;
}

namespace {

	bool isDirectory(const std::string &path)
	{
		std::filesystem::path norm_path = std::filesystem::weakly_canonical(path);
		return std::filesystem::is_directory(norm_path);
	}

	void appendTrailingSlash(const std::string &path, Request &request) {
		File file;
		file.setReturnPage({.path = path + "/", .status_code = HttpStatus::e_code::MOVED_PERMANENTLY});
		request.setFile(file);
	}
	
	bool isDirectoryRedirect(const std::string &server_root,
							const std::string &request_target,
							Request &request,
							std::string loc_path)
	{
		std::filesystem::path full =
			std::filesystem::weakly_canonical(server_root + request_target);

		bool isDir = std::filesystem::is_directory(full);

		if (request_target == loc_path || request_target == loc_path + "/") {
			isDir = true;
		}

		if (isDir && request_target.size() > 1 && request_target.back() != '/') {

			appendTrailingSlash(request_target, request);
			return true;
		}

		return false;
	}

	bool tryRelocate(Request &request, const Location &loc, const std::string &location_path, std::string &request_target) {
		
		File file;
		std::string return_path = loc.getReturnPage().path;
		HttpStatus::e_code status_code = loc.getReturnPage().status_code;
		if (return_path.empty()) return false;

		std::string remaining_path;
		if (location_path.size() <= request_target.size()) {
			remaining_path = request_target.substr(location_path.size());
		}

		std::string full_filename = return_path;
		if (remaining_path.size() > 1)
			std::string full_filename = return_path + "/" + remaining_path;

		std::cerr << "111 => " << full_filename << std::endl;
		file.setReturnPage({.path = full_filename, .status_code = status_code});
		file.setPathInfo(remaining_path);

		HttpMethodRegistry method_registry;
		file.setMethodRegistry(loc.getMethodsRegistry().value_or(method_registry));
		file.setMaxBodySize(loc.getMaxBodySize());
		file.setRelativePath(request_target);

		request.setFile(file);
		return true;
	}

	std::filesystem::path getFullFilename(Request &request, const std::string &request_target, const Location &loc, std::optional<std::string> index = std::nullopt)
	{
		std::string full_name = loc.getRoot().string() + "/" + request_target;
		std::filesystem::path norm_path = std::filesystem::weakly_canonical(full_name);

		if (request.get_method() == HttpMethod::e_code::POST) {
			return norm_path;
		}
		if (!loc.getDefaultFile().empty()) {
			return std::filesystem::weakly_canonical(full_name + "/" + loc.getDefaultFile());
		}
		else if (loc.getAutoindex() || request.get_method() == HttpMethod::e_code::POST) {
			return norm_path.string() + "/";
		} 
		else if (index.has_value()) {
			return std::filesystem::weakly_canonical(full_name + "/" + index.value());
		}
		return "";
	}

	File isMatchedDirectory(Request &request,
		const std::string &request_target,
		const Location &loc,
		ServerBlock server_block)
	{
		File file;

		std::string remaining_path = request_target.substr(loc.getPath().string().size());
		std::string full_name = loc.getRoot().string() + remaining_path;

		bool isDir = isDirectory(full_name);
		if (request_target == loc.getPath())
			isDir = true;
		if (!isDir) return file;

		std::filesystem::path full_filename_path = getFullFilename(request, remaining_path, loc, server_block._index);

		file.setIsIndex(!loc.getAutoindex() && request.get_method() != HttpMethod::e_code::POST);
		file.setIsDir(true);
		file.setAutoindex(loc.getAutoindex());
		file.setPathInfo(remaining_path);

		if (!loc.getAutoindex())
			file.setFullFilename(full_filename_path);
		else
			file.setFullFilename(full_name);

		HttpMethodRegistry method_registry;
		file.setMethodRegistry(loc.getMethodsRegistry().value_or(method_registry));
		file.setMaxBodySize(loc.getMaxBodySize());
		file.setRelativePath(request_target);

		request.setFile(file);
		return file;
	}

	bool isRegularFile(Location &loc, Request &request, std::string request_target)
	{
		File file;
		std::string remaining_path = request_target.substr(loc.getPath().string().size());
		std::string full_name = loc.getRoot().string() + "/" + remaining_path;
		std::filesystem::path norm_path_request = std::filesystem::weakly_canonical(full_name);

		bool isDir = std::filesystem::is_directory(norm_path_request);
		std::cout << norm_path_request << " " << isDir << std::endl;
		if (isDir) return false;

		file.setFullFilename(full_name);
		file.setMaxBodySize(loc.getMaxBodySize());
		file.setRelativePath(request_target);
		file.setPathInfo(remaining_path);

		HttpMethodRegistry method_registry;
		file.setMethodRegistry(loc.getMethodsRegistry().value_or(method_registry));
		request.setFile(file);

		return true;
	}

	bool isRegularFile(CGIPath &cgi, Request &request, std::string full_filename)
	{
		std::filesystem::path norm_path_request = std::filesystem::weakly_canonical(full_filename);
		File &file = request.getFile();
		bool isDir = isDirectory(norm_path_request);
		if (isDir) return false;

		std::string extension = norm_path_request.extension();
		std::string pass_to = cgi.extensions.count(extension) > 0 ? cgi.pass_to : "";
		if (!pass_to.empty()) {
			file.setExtension(extension);
			file.setPassTo(pass_to);
			request.setIsCGI(true);
			return true;
		}
	
		return false;
	}

	RequestLineValidator::e_parse_result isMatchedLocations(ServerBlock server_block, std::string &request_target, Request &request)
	{
		Log::debug("Check for matches in Location", "parser");

		Location matched_loc;
		for (auto &l : server_block._locations.value())
		{
			std::string path = l.getPath().string();
			if (path.size() > 2 &&
				path.size() >= matched_loc.getPath().string().size() &&
				!request_target.compare(0, path.size(), path)) {
					matched_loc = l;
				}
		}
		if (matched_loc.getPath().empty())
			return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;

		Log::debug("LOCATION MATCH: \n" + matched_loc.to_string(), "http-parser");
		if (tryRelocate(request, matched_loc, matched_loc.getPath(), request_target)) {
			Log::debug("Is a relocation file " + matched_loc.getPath().string(), "parser-loc");
			return RequestLineValidator::e_parse_result::RELOCATION;
		}
		if (isRegularFile(matched_loc, request, request_target)) {
			Log::debug("Is a file " + server_block._root.string() + request_target, "parser-loc");
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}
		if (isDirectoryRedirect(server_block._root.string(), request_target, request, matched_loc.getPath())) {
			Log::debug("Is a dir redir " + server_block._root.string() + request_target, "parser-loc");
			return RequestLineValidator::e_parse_result::RELOCATION;
		}
		isMatchedDirectory(request, request_target, matched_loc, server_block);
		if (!request.getFile().getFullFilename().empty()) {
			Log::debug("Is a dir " + server_block._root.string() + request_target, "parser-loc");
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}
		return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;
	}

	RequestLineValidator::e_parse_result isMatchedCGI(ServerBlock server_block, const std::filesystem::path &full_filename, Request &request)
	{
		Log::debug("Check for matches in CGI", "parser");

		std::string extension = full_filename.extension().string();
		if (extension.empty()) return RequestLineValidator::e_parse_result::MATCH_FOUND;

		CGIPath matched_cgi;
		for (auto &cgi : server_block._cgi.value())
		{
			if (cgi.extensions.count(extension)) {
				matched_cgi = cgi;
			}
		}
		if (matched_cgi.pass_to.empty())
			return RequestLineValidator::e_parse_result::MATCH_FOUND;

		Log::debug("CGI MATCH: \n" + to_string(matched_cgi), "http-parser");
		if (isRegularFile(matched_cgi, request, full_filename)) {
			return RequestLineValidator::e_parse_result::CGI;
		}

		return RequestLineValidator::e_parse_result::MATCH_FOUND;
	}

	RequestLineValidator::e_parse_result handleNoFileInConfig(ServerBlock server_block, std::string &request_target, Request &request)
	{
		File file;
		Log::debug("Handle no file in config", "parser");
		
		Location server_loc = server_block._root_restrictions;
		file.setMaxBodySize(server_loc.getMaxBodySize());
		Log::debug("server_loc: \n" + server_loc.to_string(), "http-parser");
		if (tryRelocate(request, server_loc, server_loc.getPath(), request_target)) {
			Log::debug("Is a relocation file " + server_loc.getPath().string(), "parser");
			return RequestLineValidator::e_parse_result::RELOCATION;
		}

		if (isRegularFile(server_loc, request, request_target)) {
			Log::debug("Is a regular file " + server_loc.getRoot().string() + request_target, "parser");
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}

		if (isDirectoryRedirect(server_block._root.string(), request_target, request, server_loc.getPath())) {
			Log::debug("Is a dir redir " + server_block._root.string() + request_target, "parser");
			return RequestLineValidator::e_parse_result::RELOCATION;
		}

		isMatchedDirectory(request, request_target, server_loc, server_block);
		if (!request.getFile().getFullFilename().empty()) {
			Log::debug("Is a dir " + server_loc.getRoot().string() + request_target, "parser");
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}

		std::string file_to_return = "";
		if (isDirectory(server_loc.getRoot().string() + request_target)) {
			file_to_return = server_loc.getRoot().string() + request_target + server_block._index.value_or("");
		}
		else {
			file_to_return = server_loc.getRoot().string() + request_target;
		}
		Log::debug("Tried to return: " + file_to_return, "http-parser");
		file.setFullFilename(file_to_return);
		file.setMaxBodySize(server_loc.getMaxBodySize());

		HttpMethodRegistry method_registry;
		file.setMethodRegistry(server_loc.getMethodsRegistry().value_or(method_registry));
		request.setFile(file);
		return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;
	}
}

RequestLineValidator::e_parse_result RequestLineValidator::_isRequestTargetInConfigFile(std::string &request_target)
{
	if (!_parse_context.request.getServerBlock()) return UNKNOWN_ERROR;

	const ServerBlock &server_block = *_parse_context.request.getServerBlock();
	Request &request = _parse_context.request;

	std::filesystem::path filename_path(request_target);
	std::string extension = filename_path.extension().string();
	Log::debug("Filename: " + filename_path.string() + " Extension: " + extension, "http-parser");

	if (isDirectoryRedirect(server_block._root.string(), request_target, request, "")) 
		return RELOCATION;

	e_parse_result res;
	if (server_block._locations.has_value())
	{
		res = isMatchedLocations(server_block, request_target, request);
		if (res == RELOCATION) return res;
		if (res == NO_FILE_IN_CONFIG)
			handleNoFileInConfig(server_block, request_target, request);
	}
	std::cout << request.getFile();
	if (server_block._cgi.has_value())
	{
		res = isMatchedCGI(server_block, request.getFile().getFullFilename(), request);
		if (res != NO_FILE_IN_CONFIG) return res;
	}
	return res;
}

void RequestLineValidator::parse()
{
	std::string buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);

	if (!_isValidRequestLine(buffer)){
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		std::cerr << _parse_context.request.get_status_code() << " - request line is invalid" << std::endl;
		return ;
	}
	_parse_context.request.set_method(_parse_context.request.get_header_value(http::headers::METHOD));
	std::string request_parser = _parse_context.request.get_header_value(http::headers::REQUEST_TARGET);
	std::string decoded_path = PercentEncoder::percent_encoding(request_parser);

	e_parse_result result = _isRequestTargetInConfigFile(decoded_path);
	if (result == UNKNOWN_ERROR) {
		_parse_context.request.set_status_code(HttpStatus::e_code::SERVICE_UNAVAILABLE);
		Log::warning("Server block null reference.");
		return ;
	}

	if (result == NO_FILE_IN_CONFIG) {
		_parse_context.request.set_status_code(HttpStatus::e_code::NOT_FOUND);
		Log::warning("Request target is not in configuration file.");
		return ;
	}

	if (result == RELOCATION) {
		_parse_context.request.set_status_code(_parse_context.request.getFile().getReturnPage().status_code);
		Log::info("Request target relocates to new location. " + std::to_string(static_cast<int>(_parse_context.request.get_status_code())));
		return ;
	}

	_parse_context.request.set_header_value(http::headers::REQUEST_TARGET_DECODED, decoded_path);

	if (!_isValidHttpVersion()
		|| !_isValidUriLength()
		|| !_isMethodAllowed()) {

			Log::warning("Something is wrong " + std::to_string(static_cast<int>(_parse_context.request.get_status_code())));
			return ;
		}

	_parse_context.request.set_status_code(HttpStatus::e_code::OK);
}
