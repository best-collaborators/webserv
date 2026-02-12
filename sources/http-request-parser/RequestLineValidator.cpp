#include "RequestLineValidator.hpp"

RequestLineValidator::	RequestLineValidator( std::string &raw_bits, Request &request ) :
	_raw_bits(raw_bits), _request(request) { }

bool RequestLineValidator::add_value_to_map(
	std::regex reg_method,
	std::string errmsg,
	std::string key
)
{
	std::string value;
	if (!RequestStringUtils::tryExtractHeaderField(value, _buffer, reg_method, errmsg)) {
		return false;
	}
	_request.set_header_value(key, value);
	return true;
}

bool RequestLineValidator::is_valid_request_line()
{
	if (_buffer.length() > http::limits::max_header_value_length) return false;

	return (add_value_to_map(HttpRegexPatterns::METHOD(), ERROR_HTTP_METHOD, "method")
	&& add_value_to_map(HttpRegexPatterns::REQUEST_TARGET(), ERROR_HTTP_REQUEST_TARGET, "request-target")
	&& add_value_to_map(HttpRegexPatterns::VERSION(), ERROR_HTTP_VESRION, "version"));
}

HttpStatus::e_code RequestLineValidator::validate()
{
	_buffer = RequestStringUtils::cut_after_new_line(_raw_bits);

	if (_buffer == "") return HttpStatus::code_from_number(400);

	if (is_valid_request_line() == false){
		std::cerr << "400 Bad Request - request line is invalid" << std::endl;
		return HttpStatus::code_from_number(400);
	}

	std::string decoded_path = PercentEncoder::percent_encoding(_request.get_header_value("request-target"));
	_request.set_header_value("request-target-decoded", decoded_path);

	if (_request.get_header_value("version") != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return HttpStatus::code_from_number(505);
	}

	if (_request.get_header_value("request-target-decoded").length() > http::limits::max_uri_length){
		std::cerr << "414 URI Too Long" << std::endl;
		return HttpStatus::code_from_number(414);
	}

	if (HttpMethod::isAllowed(_request.get_header_value("method"))) {
		std::cerr << "405 Not Allowed" << std::endl;
		return HttpStatus::code_from_number(405);
	}

	return HttpStatus::code_from_number(0);
}
