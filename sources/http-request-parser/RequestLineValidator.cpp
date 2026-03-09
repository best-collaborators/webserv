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

#include "File.hpp"
RequestLineValidator::e_parse_result RequestLineValidator::_isRequestTargetInConfigFile(std::string &request_target) 
{
	const ServerBlock &server_block = _parse_context.request.getServerBlock();

	File file;
	std::filesystem::path filename_path(request_target);
	std::string filename = filename_path.filename().string();
	std::string extension = filename_path.filename().extension().string();
	std::cout << "Filename: " << filename_path.string() << " Extension: " << extension << std::endl;

	bool isDir = std::filesystem::is_directory(server_block._root.string() + request_target);
	if (isDir && *request_target.end() != '/') {
		std::cout <<  request_target + "/" << std::endl;
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

				if (!l.getReturnPage().path.empty()) {
					std::string full_filename = std::string(l.getReturnPage().path) + "/" + request_target.substr(path.size());
					file.setReturnPage({.path = full_filename, .status_code = l.getReturnPage().status_code});
					std::cout << "File path for relocation is: " << full_filename << std::endl;
					std::cout << "File status code for relocation is: " <<  l.getReturnPage().status_code << std::endl;
					_parse_context.request.setFile(file);
					return RELOCATION;
				}

				if (isDir) {
					std::string full_filename = std::string(l.getRoot()) + "/" + l.getDefaultFile();
					file.setIsDir(true);
					file.setAutoindex(l.getAutoindex());
					
					file.setFullFilename(full_filename);
				}
				else if (request_target[path.size()] != '/') {
					continue;
				}
				else {
					std::string full_filename = std::string(l.getRoot()) + "/" + filename;
					//! Temporary fix
					request_target = full_filename;
					file.setFullFilename(full_filename);
					file.setExtension(extension);
				}
				matched_loc = l;
			}
		}
		if (!matched_loc.getPath().empty()) {
			std::cout << "LOCATION MATCH: \n" << matched_loc << std::endl;
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
				}
			matched_cgi = cgi;
		}

		if (!matched_cgi.path.empty()) {
			std::cout << "CGI MATCH: \n" << matched_cgi.path << std::endl;
			if (file.getPassTo().has_value())
				_parse_context.request.setIsCGI(true);
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
