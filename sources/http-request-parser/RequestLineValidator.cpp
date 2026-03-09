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
	&& _addValueToMap(HttpRegexPatterns::VERSION(), buffer, ERROR_HTTP_VESRION, "version"));
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
	std::cout << _parse_context.request.get_header_value(http::headers::METHOD) << std::endl;
	// std::cout << HttpMethodRegistry::isAllowed(_parse_context.request.get_header_value(http::headers::METHOD)) << std::endl;
	// if (!HttpMethodRegistry::isAllowed(_parse_context.request.get_header_value(http::headers::METHOD))) {
	// 	_parse_context.request.set_status_code(HttpStatus::e_code::METHOD_NOT_ALLOWED);
	// 	std::cerr << _parse_context.request.get_status_code() << std::endl;
	// 	return false;
	// }
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
		return request_target.size() > 1 && *(request_target.end() - 1) == '/';
	}

	
}

#include "File.hpp"

RequestLineValidator::e_parse_result RequestLineValidator::_isRequestTargetInConfigFile(std::string &request_target) 
{
	const ServerBlock &server_block = _parse_context.request.getServerBlock();

	File file;
	std::filesystem::path filename_path(request_target);
	std::string extension = filename_path.extension().string();
	std::cout << "Filename: " << filename_path.string() << " Extension: " << extension << std::endl;

	bool isDir = isDirectory(server_block._root.string() + request_target);
	if (isDir && hasTrailingSlash(request_target)) {
		file.setReturnPage({.path = request_target + "/", .status_code = HttpStatus::e_code::MOVED_PERMANENTLY});
		_parse_context.request.setFile(file); 
		return RELOCATION;
	}

	if (server_block._locations.has_value())
	{
		Location matched_loc;
		for (auto &l : server_block._locations.value())
		{
			std::string path = l.getPath().string();
			if (!request_target.compare(0, path.size(), path)) {

				if (path.size() > 1 && request_target[path.size()] != '/') {
					continue;
				}

				if (!l.getReturnPage().path.empty()) {
					std::string full_filename = std::string(l.getReturnPage().path) + "/" + request_target.substr(path.size());
					file.setReturnPage({.path = full_filename, .status_code = l.getReturnPage().status_code});
					std::cout << "File path for relocation is: " << full_filename << std::endl;
					std::cout << "File status code for relocation is: " <<  l.getReturnPage().status_code << std::endl;
					_parse_context.request.setFile(file);
					return RELOCATION;
				}

				if (isDir) {
					
					std::string request_target_without_path = request_target.substr(path.size());
					std::string full_filename;
					if (!l.getDefaultFile().empty())
						full_filename = std::string(l.getRoot()) + "/" + request_target_without_path + l.getDefaultFile();
					else if (server_block._index.has_value())
						full_filename = std::string(l.getRoot()) + "/" + request_target_without_path + server_block._index.value();

					std::filesystem::path normalized_path = std::filesystem::weakly_canonical(full_filename);
					std::cout << "normalized_path: " << normalized_path << std::endl;
					std::cout << "request_target: " << request_target << std::endl;
					std::cout << "request_target_without_path: " << request_target_without_path << std::endl;
					std::cout << "PATH: " << path << std::endl;

					file.setIsDir(true);
					file.setAutoindex(l.getAutoindex());
					file.setFullFilename(normalized_path.string());
				}
				else {
					std::string request_target_without_path = request_target.substr(path.size());
					std::string full_filename = std::string(l.getRoot()) + "/" + request_target_without_path;
					std::filesystem::path path = std::filesystem::weakly_canonical(full_filename);

					std::cout << "PATH: " << path << std::endl;
					//! Temporary fix
					// request_target = full_filename;
					file.setFullFilename(path);
					file.setExtension(extension);
				}
				matched_loc = l;
			}
		}
		if (!matched_loc.getPath().empty()) {
			std::cout << "LOCATION MATCH: \n" << matched_loc << std::endl;

			std::filesystem::path norm_path_location = std::filesystem::weakly_canonical(file.getFullFilename());
			isDir = std::filesystem::is_directory(norm_path_location);
			std::cout <<  norm_path_location.string() << std::endl;
			std::cout <<  isDir << std::endl;

			if (isDir && *(file.getFullFilename().end() - 1) != '/') {
				file.setReturnPage({.path = request_target + "/", .status_code = HttpStatus::e_code::MOVED_PERMANENTLY});
				_parse_context.request.setFile(file);
				return RELOCATION;
			}

			bool isFile = std::filesystem::is_regular_file(file.getFullFilename());
			if (isFile) {
				file.setFullFilename(file.getFullFilename());
				_parse_context.request.setFile(file);
				return MATCH_FOUND;
			}
			_parse_context.request.setFile(file);
			return MATCH_FOUND;
		}
	}

	if (server_block._cgi.has_value())
	{
		CGIPath matched_cgi;
		for (auto &cgi : server_block._cgi.value())
		{
			std::string path = cgi.path;
			if (!request_target.compare(0, path.size(), path)) {

				if (request_target[path.size()] != '/') {
					continue;
				}
				std::string full_filename = std::string(server_block._root) + request_target;
				request_target = full_filename;
				file.setFullFilename(request_target);
				file.setExtension(extension);

				std::string pass_to = cgi.pass_to.count(extension) > 0 ? cgi.pass_to.at(extension) : "";
				if (!pass_to.empty())
					file.setPassTo(pass_to);
				matched_cgi = cgi;
			}
		}

		if (!matched_cgi.path.empty()) {
			std::cout << "CGI MATCH: \n" << matched_cgi.path << std::endl;
			if (file.getPassTo().has_value())
				_parse_context.request.setIsCGI(true);
			_parse_context.request.setFile(file);
			return MATCH_FOUND;
		}
	}

	std::string full_filename = server_block._root.string() + request_target;
	std::filesystem::path norm_path_request = std::filesystem::weakly_canonical(full_filename);

	bool isFile = std::filesystem::is_regular_file(full_filename);
	if (isFile) {
		file.setFullFilename(full_filename);
		_parse_context.request.setFile(file);
		return MATCH_FOUND;
	}

	isDir = std::filesystem::is_directory(norm_path_request);
	if (isDir && *(request_target.end() - 1) != '/') {
		std::cout <<  request_target + "/" << std::endl;
		file.setReturnPage({.path = norm_path_request, .status_code = HttpStatus::e_code::MOVED_PERMANENTLY});
		_parse_context.request.setFile(file);
		return RELOCATION;
	}

	if ( server_block._index.has_value()) {
		std::string full_filename = server_block._root.string() + request_target + server_block._index.value();
		_parse_context.request.setFile(file);
		bool isFile = std::filesystem::is_regular_file(full_filename);
		if (isFile) {
			file.setFullFilename(full_filename);
			_parse_context.request.setFile(file);
			return MATCH_FOUND;
		}
	}

	return NO_FILE_IN_CONFIG;
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
