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

	return (_addValueToMap(HttpRegexPatterns::METHOD(), buffer, ERROR_HTTP_METHOD, http::headers::METHOD)
	&& _addValueToMap(HttpRegexPatterns::FILEPATH(), buffer, ERROR_HTTP_REQUEST_TARGET, http::headers::REQUEST_TARGET)
	&& _addValueToMap(HttpRegexPatterns::VERSION(), buffer, ERROR_HTTP_VERSION, "version"));
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

	bool hasTrailingSlash(const std::string &request_target) {
		return *(request_target.end() - 1) == '/';
	}

	void appendTrailingSlash(const std::string &path, Request &request) {
		File file;
		file.setReturnPage({.path = path + "/", .status_code = HttpStatus::e_code::MOVED_PERMANENTLY});
		request.setFile(file);
	}

	bool isDirectoryRedirect(const std::string &server_root, const std::string &request_target, Request &request)
	{
		bool isDir = isDirectory(server_root + request_target);
		if (isDir && request_target.size() > 1 && !hasTrailingSlash(request_target)) {

			appendTrailingSlash(request_target, request);
			return true;
		}
		return false;
	}

	bool tryRelocate(const Location &location, const std::string &location_path, std::string &request_target) {
		
		File file;
		std::string return_path = location.getReturnPage().path;
		HttpStatus::e_code status_code = location.getReturnPage().status_code;
		if (return_path.empty()) return false;

		std::string remaining_path;
		if (location_path.size() <= request_target.size()) {
			remaining_path = request_target.substr(location_path.size());
		}

		std::string full_filename = return_path;
		if (remaining_path.size() > 1)
			std::string full_filename = return_path + "/" + remaining_path;

		file.setReturnPage({.path = full_filename, .status_code = status_code});
		return true;
	}

	std::filesystem::path getFullFilename(Request &request, const std::string &request_target, const Location &loc, std::optional<std::string> index = std::nullopt)
	{
		std::string full_name = loc.getRoot().string() + "/" + request_target;
		std::filesystem::path norm_path = std::filesystem::weakly_canonical(full_name);
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

		Log::warning(loc.getRoot());
		std::string remaining_path = request_target.substr(loc.getPath().string().size());
		std::string full_name = loc.getRoot().string() + remaining_path;

		std::cout << full_name << std::endl;
		if (!isDirectory(full_name)) return file;

		std::filesystem::path full_filename_path = getFullFilename(request, remaining_path, loc, server_block._index);

		Log::critical(full_filename_path);

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
		std::string full_name = loc.getRoot().string() + remaining_path;
		std::filesystem::path norm_path_request = std::filesystem::weakly_canonical(full_name);

		bool isDir = std::filesystem::is_directory(norm_path_request);
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

	bool isRegularFile(ServerBlock &server_block, CGIPath &cgi, Request &request, std::string request_target)
	{
		File file;
		std::string remaining_path = request_target.substr(cgi.path.string().size());
		std::string full_name = server_block._root.string() + "/" + remaining_path;
		std::filesystem::path norm_path_request = std::filesystem::weakly_canonical(full_name);

		bool isDir = isDirectory(norm_path_request);
		if (isDir) return false;

		file.setFullFilename(full_name);
		file.setMaxBodySize(cgi.max_body_size);
		file.setPathInfo(remaining_path);

		HttpMethodRegistry method_registry;
		file.setMethodRegistry(cgi.methods_registry.value_or(method_registry));

		std::string extension = norm_path_request.extension();
		file.setExtension(extension);

		std::string pass_to = cgi.extensions.count(extension) > 0 ? cgi.pass_to : "";
		if (!pass_to.empty()) {
			request.setIsCGI(true);
			file.setRelativePath(request_target);
			file.setPassTo(pass_to);
			request.setFile(file);
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
				path.size() > matched_loc.getPath().string().size() &&
				!request_target.compare(0, path.size(), path)) {
					matched_loc = l;
				}
		}
		if (matched_loc.getPath().empty())
			return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;

		Log::debug("LOCATION MATCH: \n" + matched_loc.to_string(), "http-parser");
		if (tryRelocate(matched_loc, matched_loc.getPath(), request_target)) {
			return RequestLineValidator::e_parse_result::RELOCATION;
		}

		if (isRegularFile(matched_loc, request, request_target)) {
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}

		if (isDirectoryRedirect(server_block._root.string(), request_target, request)) {
			return RequestLineValidator::e_parse_result::RELOCATION;
		}

		isMatchedDirectory(request, request_target, matched_loc, server_block);
		if (!request.getFile().getFullFilename().empty()) {
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}
		return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;
	}

	RequestLineValidator::e_parse_result isMatchedCGI(ServerBlock server_block, std::string &request_target, Request &request)
	{
		Log::debug("Check for matches in CGI", "parser");
		CGIPath matched_cgi;
		for (auto &cgi : server_block._cgi.value())
		{
			std::string path = cgi.path;
			if (path.size() >= matched_cgi.path.string().size()
				&& !request_target.compare(0, path.size(), path)) {
					matched_cgi = cgi;
				}
		}
		if (matched_cgi.pass_to.empty())
			return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;

		if (!matched_cgi.extensions.count(std::filesystem::path(request_target).extension())) {
			return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;
		}
		Log::debug("CGI MATCH: \n" + to_string(matched_cgi), "http-parser");
		if (isRegularFile(server_block, matched_cgi, request, request_target)) {
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}

		return RequestLineValidator::e_parse_result::NO_FILE_IN_CONFIG;
	}

	RequestLineValidator::e_parse_result handleNoFileInConfig(ServerBlock server_block, std::string &request_target, Request &request)
	{
		File file;
		Log::debug("Handle no file in config", "parser");
		
		Location server_loc = server_block._root_restrictions;
		file.setMaxBodySize(server_loc.getMaxBodySize());
		Log::debug("server_loc: \n" + server_loc.to_string(), "http-parser");
		if (tryRelocate(server_loc, server_loc.getPath(), request_target)) {
			Log::debug("Is a relocation file " + server_loc.getPath().string(), "parser");
			return RequestLineValidator::e_parse_result::RELOCATION;
		}

		if (isRegularFile(server_loc, request, request_target)) {
			Log::debug("Is a regular file " + server_loc.getRoot().string() + request_target, "parser");
			return RequestLineValidator::e_parse_result::MATCH_FOUND;
		}

		if (isDirectoryRedirect(server_block._root.string(), request_target, request)) {
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

	if (isDirectoryRedirect(server_block._root.string(), request_target, request)) 
		return RELOCATION;

	if (server_block._cgi.has_value())
	{
		e_parse_result res = isMatchedCGI(server_block, request_target, request);
		if (res != NO_FILE_IN_CONFIG) return res;
	}

	if (server_block._locations.has_value())
	{
		e_parse_result res = isMatchedLocations(server_block, request_target, request);
		if (res != NO_FILE_IN_CONFIG) return res;
	}

	return handleNoFileInConfig(server_block, request_target, request);
}

void RequestLineValidator::parse()
{
	std::string buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);

	if (!_isValidRequestLine(buffer)){
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		std::cerr << _parse_context.request.get_status_code() << " - request line is invalid" << std::endl;
		return ;
	}

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

			return ;
		}

	_parse_context.request.set_status_code(HttpStatus::e_code::OK);
}
