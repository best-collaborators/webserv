#include "HttpHeaderParser.hpp"

HttpHeaderParser::HttpHeaderParser( std::string &raw_bits, Request &request )
: _raw_bits(raw_bits), _request(request) { }

bool HttpHeaderParser::_isValidHeader()
{
	std::string name;
	if (!RequestStringUtils::tryExtractHeaderField(name, _buffer, HttpRegexPatterns::HEADER(), "[parser] Http header is invalid.")) {
		return false;
	}

	if (name.length() > http::limits::max_header_value_length) return false;

	name.erase(name.length() - 1);
	RequestStringUtils::transform_to_lower(name);

	if ((name == "host" || name == "content-length") && _buffer.empty())
		return false;

	if (_request.get_header_count(name)) {
		if (name == "host" || name == "content-length") {
			std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
			return false;
		}
		else {
			if (_request.get_header_value(name) != _buffer)
			_request.append_header_value(name, _buffer);
			return true;
		}
	}

	_request.set_header_value(name, _buffer);
	return true;
}

HttpStatus::e_code HttpHeaderParser::_validateRequestHeaders()
{
	_buffer = RequestStringUtils::cut_after_new_line(_raw_bits);
	while (!_buffer.empty()) {
		
		if (!_isValidHeader()) {
			std::cerr << "400 Bad Request - header is invalid" << std::endl;
			return HttpStatus::code_from_number(400);
		}

		if (_request.amount_of_headers() >= http::limits::max_header_count) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl;
			return HttpStatus::code_from_number(400);
		}

		if (_request.get_header_count("content-length")) {
			HttpStatus::e_code content_length_status = _contentLengthValidation();
			if (HttpStatus::is_bad(content_length_status)) {
				return content_length_status;
			}
		}
		_buffer = RequestStringUtils::cut_after_new_line(_raw_bits);
	}

	_request.set_method(_request.get_header_value("method"));
	if (_request.get_method() == HttpMethod::e_code::POST
		&& !_request.get_header_count("content-length")
		&& !_request.get_header_count("transfer-encoding")) {

		std::cerr << "411 Length Required" << std::endl;
		return HttpStatus::code_from_number(411);
	}

	return HttpStatus::code_from_number(200);
}

HttpStatus::e_code HttpHeaderParser::parse()
{
	if (_raw_bits.empty())
		RequestGenerator::create_post_request(_raw_bits);

	HttpStatus::e_code headers_validation_status = _validateRequestHeaders();
	if (HttpStatus::is_bad(headers_validation_status)) {
		_request.set_status_code(headers_validation_status);
		return headers_validation_status;
	}

	_request.set_status_code(200);
	return HttpStatus::code_from_number(200);
}

HttpStatus::e_code HttpHeaderParser::_contentLengthValidation(){

	if (_request.get_header_count("transfer-encoding")) {
		std::cerr << "400 Bad Request transfer-encoding + content-length" << std::endl;
		return HttpStatus::code_from_number(400);
	}
	try {
		size_t pos;
		const std::string content_length_str = _request.get_header_value("content-length");
		int test_length = std::stoll(content_length_str, &pos, 10);
		if (content_length_str.length() != pos) {
			std::cerr << "400 Bad Request - content-length is NAN" << std::endl; 
			return HttpStatus::code_from_number(400);
		}

		// max size is 1mb = 1048576b
		//! REQUEST TOO LARGE - REMOVE
		if (test_length < 0) {
			std::cerr << "413 Request Entity Too Large" << std::endl;
			return HttpStatus::code_from_number(413);
		}

	}
	catch(const std::exception& e) {
		std::cerr << "400 Bad Request - content-length is NAN" << std::endl;
		return HttpStatus::code_from_number(413);
	}
	return HttpStatus::code_from_number(200);
}
