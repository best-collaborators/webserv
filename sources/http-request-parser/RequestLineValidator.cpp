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
	if (buffer.length() > http::limits::max_header_value_length) return false;

	return (_addValueToMap(HttpRegexPatterns::METHOD(), buffer, ERROR_HTTP_METHOD, "method")
	&& _addValueToMap(HttpRegexPatterns::REQUEST_TARGET(), buffer, ERROR_HTTP_REQUEST_TARGET, "request-target")
	&& _addValueToMap(HttpRegexPatterns::VERSION(), buffer, ERROR_HTTP_VESRION, "version"));
}

HttpStatus::e_code RequestLineValidator::validate()
{
	std::string buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);

	if (buffer.empty()) return HttpStatus::code_from_number(400);

	if (!_isValidRequestLine(buffer)){
		std::cerr << "400 Bad Request - request line is invalid" << std::endl;
		return HttpStatus::code_from_number(400);
	}

	std::string decoded_path = PercentEncoder::percent_encoding(_parse_context.request.get_header_value("request-target"));
	_parse_context.request.set_header_value("request-target-decoded", decoded_path);

	if (_parse_context.request.get_header_value("version") != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return HttpStatus::code_from_number(505);
	}

	if (_parse_context.request.get_header_value("request-target-decoded").length() > http::limits::max_uri_length){
		std::cerr << "414 URI Too Long" << std::endl;
		return HttpStatus::code_from_number(414);
	}

	if (HttpMethod::isAllowed(_parse_context.request.get_header_value("method"))) {
		std::cerr << "405 Not Allowed" << std::endl;
		return HttpStatus::code_from_number(405);
	}

	return HttpStatus::code_from_number(200);
}
