#include "RequestLineValidator.hpp"

RequestLineValidator::RequestLineValidator(/* args */)
{
}

RequestLineValidator::~RequestLineValidator()
{
}

bool RequestLineValidator::is_valid_request_line()
{
	if (_buffer.length() > 8192) return false;
	return (add_value_to_map(REGEX_HTTP_METHOD, ERROR_HTTP_METHOD, "method")
	&& add_value_to_map(REGEX_HTTP_REQUEST_TARGET, ERROR_HTTP_REQUEST_TARGET, "request-target")
	&& add_value_to_map(REGEX_HTTP_VESRION, ERROR_HTTP_VESRION, "version"));
}

uint RequestLineValidator::validate_request_line()
{
	_buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);

	if (_buffer == "") return 400;
	if (is_valid_request_line() == false){
		std::cerr << "400 Bad Request - request line is invalid" << std::endl;
		return 400;
	}

	if (_request.get_header_value("version") != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return 505;
	}

	if (_request.get_header_value("request-target").length() > 4096){
		std::cerr << "414 URI Too Long" << std::endl;
		return 414;
	}

	if (_request.get_header_value("method") != "GET"
		&& _request.get_header_value("method") != "POST"
		&& _request.get_header_value("method") != "OPTIONS"
		&& _request.get_header_value("method") != "DELETE") {
		std::cerr << "405 Not Allowed" << std::endl;
		return 405;
	}

	return 0;
}
