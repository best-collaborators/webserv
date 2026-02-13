#include "HttpHeaderParser.hpp"

HttpHeaderParser::HttpHeaderParser( ParseContext &parse_context )
: _parse_context(parse_context) { }

bool HttpHeaderParser::_isValidHeader(std::string &buffer)
{
	std::string name;
	if (!RequestStringUtils::tryExtractHeaderField(name, buffer, HttpRegexPatterns::HEADER(), "[parser] Http header is invalid.")) {
		return false;
	}

	if (name.length() > http::limits::max_header_value_length) return false;

	name.erase(name.length() - 1);
	RequestStringUtils::transform_to_lower(name);

	if ((name == "host" || name == "content-length") && buffer.empty())
		return false;

	if (_parse_context.request.get_header_count(name)) {
		if (name == "host" || name == "content-length") {
			std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
			return false;
		}
		else {
			if (_parse_context.request.get_header_value(name) != buffer)
			_parse_context.request.append_header_value(name, buffer);
			return true;
		}
	}

	_parse_context.request.set_header_value(name, buffer);
	return true;
}

HttpStatus::e_code HttpHeaderParser::_validateRequestHeaders()
{
	std::string buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	while (!buffer.empty()) {
		
		if (!_isValidHeader(buffer)) {
			std::cerr << "400 Bad Request - header is invalid" << std::endl;
			return HttpStatus::code_from_number(400);
		}

		if (_parse_context.request.amount_of_headers() >= http::limits::max_header_count) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl;
			return HttpStatus::code_from_number(400);
		}

		if (_parse_context.request.get_header_count("content-length")) {
			HttpStatus::e_code content_length_status = _contentLengthValidation();
			if (HttpStatus::is_bad(content_length_status)) {
				return content_length_status;
			}
		}
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	}

	_parse_context.request.set_method(_parse_context.request.get_header_value("method"));
	if (_parse_context.request.expectsBody() && !_parse_context.request.has_body_required_headers()) {

		std::cerr << "411 Length Required" << std::endl;
		return HttpStatus::code_from_number(411);
	}

	return HttpStatus::code_from_number(200);
}

HttpStatus::e_code HttpHeaderParser::parse()
{
	if (_parse_context.raw_bits.empty())
		RequestGenerator::create_post_request(_parse_context.raw_bits);

	HttpStatus::e_code headers_validation_status = _validateRequestHeaders();
	if (HttpStatus::is_bad(headers_validation_status)) {
		return headers_validation_status;
	}

	return HttpStatus::code_from_number(200);
}

HttpStatus::e_code HttpHeaderParser::_contentLengthValidation(){

	if (_parse_context.request.get_header_count("transfer-encoding")) {
		std::cerr << "400 Bad Request transfer-encoding + content-length" << std::endl;
		return HttpStatus::code_from_number(400);
	}
	try {
		size_t pos;
		const std::string content_length_str = _parse_context.request.getContentType();
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
