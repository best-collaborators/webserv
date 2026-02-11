#include "RequestLineValidator.hpp"

RequestLineValidator::	RequestLineValidator( std::string &raw_bits, Request &request ) :
	_raw_bits(raw_bits), _request(request) { }

bool RequestLineValidator::add_value_to_map(
	std::regex reg_method,
	std::string errmsg,
	std::string key
)
{
	std::string method = RegexMatcher::get_regex_value(_buffer, reg_method);
	if (method == "") {
		std::cout << errmsg << std::endl;
		return false;
	}
	method = Trimmer::trim(method);
	PercentEncoder::percent_encoding(method);
	_request.set_header_value(key, method);
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

	if (_request.get_header_value("version") != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return HttpStatus::code_from_number(505);
	}

	if (_request.get_header_value("request-target").length() > http::limits::max_uri_length){
		std::cerr << "414 URI Too Long" << std::endl;
		return HttpStatus::code_from_number(414);
	}

	if (_request.get_header_value("method") != "GET"
		&& _request.get_header_value("method") != "POST"
		&& _request.get_header_value("method") != "OPTIONS"
		&& _request.get_header_value("method") != "DELETE") {
		std::cerr << "405 Not Allowed" << std::endl;
		return HttpStatus::code_from_number(405);
	}

	return HttpStatus::code_from_number(0);
}
