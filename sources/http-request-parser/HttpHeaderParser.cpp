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

	if ((name == http::headers::HOST || name == http::headers::CONTENT_LENGTH) && buffer.empty())
		return false;

	if (_parse_context.request.get_header_count(name)) {
		if (name == http::headers::HOST
			|| name == http::headers::CONTENT_LENGTH
			|| name == http::headers::CONTENT_TYPE) {

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
			return HttpStatus::e_code::BAD_REQUEST;
		}

		if (_parse_context.request.amount_of_headers() >= http::limits::max_header_count) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl;
			return HttpStatus::e_code::REQUEST_HEADER_FIELDS_TOO_LARGE;
		}

		if (_parse_context.request.get_header_count(http::headers::CONTENT_LENGTH)) {
			HttpStatus::e_code content_length_status = _contentLengthValidation();
			if (HttpStatus::is_bad(content_length_status)) {
				return content_length_status;
			}
		}
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	}

	_parse_context.request.set_method(_parse_context.request.get_header_value(http::headers::METHOD));
	if (_parse_context.request.getBodyStatus() != RequestType::NO_BODY
		&& !_parse_context.request.has_body_required_headers()) {

		std::cerr << "411 Length Required" << std::endl;
		return HttpStatus::e_code::LENGTH_REQUIRED;
	}

	return HttpStatus::e_code::OK;
}

void HttpHeaderParser::parse()
{
	HttpStatus::e_code headers_validation_status = _validateRequestHeaders();
	if (HttpStatus::is_bad(headers_validation_status)) {
		_parse_context.request.set_status_code(headers_validation_status);
		return ;
	}
	_parse_context.request.set_status_code(HttpStatus::e_code::OK);
}

HttpStatus::e_code HttpHeaderParser::_contentLengthValidation(){

	if (_parse_context.request.get_header_count(http::headers::TRANSFER_ENCODING)) {
		std::cerr << "400 Bad Request transfer-encoding + content-length" << std::endl;
		return HttpStatus::e_code::BAD_REQUEST;
	}
	try {
		size_t pos;
		const std::string content_length_str = _parse_context.request.get_header_value(http::headers::CONTENT_LENGTH);
		size_t test_length = std::stoull(content_length_str, &pos, 10);
		if (content_length_str.length() != pos) {
			std::cerr << "400 Bad Request - content-length is NAN" << std::endl; 
			return HttpStatus::e_code::BAD_REQUEST;
		}

		if (test_length > _parse_context.request.getFile().getMaxBodySize()) {
			std::cerr << "413 Request Entity Too Large" << std::endl;
			return HttpStatus::e_code::CONTENT_TOO_LARGE;
		}
	}
	catch(const std::exception& e) {
		std::cerr << "400 Bad Request - content-length is NAN" << std::endl;
		return HttpStatus::e_code::BAD_REQUEST;
	}

	return HttpStatus::e_code::OK;
}

