#include "HttpHeaderParser.hpp"

namespace {
	bool containsInvalidChars(const std::string& line) {
		for (unsigned char c : line) {
			if ((c <= 0x1F && c != '\t') || c == 0x7F) {
				return true;
			}

			if (c > 0x7F) {
				return true;
			}
		}
		return false;
	}

	bool isValidHeaderNameChar(char c) {
		return std::isalnum(static_cast<unsigned char>(c)) ||
			c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
			c == '\''|| c == '*' || c == '+' || c == '-' || c == '.' ||
			c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
	}

	std::pair<std::string, std::string> checkHeaderFormatValidity(const std::string &line)
	{
		std::pair<std::string, std::string> key_value = {};
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos) {
			return key_value;
		}

		std::string name = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		for (char c : name) {
			if (!isValidHeaderNameChar(c)) {
				return key_value;
			}
		}
		if (containsInvalidChars(line)) {
			return key_value;
		}
		key_value.first = name;
		key_value.second = value;
		return key_value;
	}
}

HttpHeaderParser::HttpHeaderParser( ParseContext &parse_context )
: _parse_context(parse_context) { }

HttpHeaderParser::HttpHeaderParser( ParseContext &parse_context, size_t max_body_size )
: _parse_context(parse_context), _max_body_size(max_body_size) { }

bool HttpHeaderParser::_isValidHeaderLength(const std::string &buffer) const {
	return buffer.length() <= http::limits::max_header_value_length;
}

std::pair<std::string, std::string> HttpHeaderParser::_parseHeader(const std::string &buffer) const {
	return checkHeaderFormatValidity(buffer);
}

void HttpHeaderParser::_normalizeHeader(std::string &name, std::string &value) const {
	RequestStringUtils::transform_to_lower(name);
	Trimmer::trim(value);
	RequestStringUtils::transform_to_lower(value);
}

bool HttpHeaderParser::_validateSpecialHeaders(const std::string &name,
											   const std::string &value,
											   const std::string &buffer) const
{
	if (name == http::headers::TRANSFER_ENCODING && value != "chunked")
		return false;

	if ((name == http::headers::HOST || name == http::headers::CONTENT_LENGTH) && buffer.empty())
		return false;

	return true;
}

bool HttpHeaderParser::_handleDuplicates(const std::string &name,
										const std::string &buffer)
{
	if (_parse_context.request.get_header_count(name) == 0)
		return true;

	if (_isCriticalHeader(name)) {
		std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
		return false;
	}

	if (_parse_context.request.get_header_value(name) != buffer)
		_parse_context.request.append_header_value(name, buffer);

	return true;
}

bool HttpHeaderParser::_isCriticalHeader(const std::string &name) const {
	return name == http::headers::HOST
		|| name == http::headers::CONTENT_LENGTH
		|| name == http::headers::CONTENT_TYPE;
}

bool HttpHeaderParser::_isValidHeader(std::string &buffer)
{
	if (!_isValidHeaderLength(buffer))
		return false;

	auto key_value = _parseHeader(buffer);
	std::string &name = key_value.first;
	std::string &value = key_value.second;
	if (name.empty() || value.empty()) return false;

	_normalizeHeader(name, value);

	if (!_validateSpecialHeaders(name, value, buffer))
		return false;

	if (!_handleDuplicates(name, buffer))
		return false;

	_parse_context.request.set_header_value(name, value);
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
		if (!HttpStatus::is_redirect(_parse_context.request.get_status_code()))
			_parse_context.request.set_status_code(headers_validation_status);
		return ;
	}
	if (!HttpStatus::is_redirect(_parse_context.request.get_status_code()))
		_parse_context.request.set_status_code(HttpStatus::e_code::OK);
}

HttpStatus::e_code HttpHeaderParser::_contentLengthValidation(){

	if (_parse_context.request.get_header_count(http::headers::TRANSFER_ENCODING)) {
		Log::error("400 Bad Request transfer-encoding + content-length", "parser");
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

		size_t body_size = std::max(_max_body_size, _parse_context.request.getFile().getMaxBodySize());
		if (test_length > body_size) {
			std::cerr << "413 Request Entity Too Large: " << test_length << " out " << _parse_context.request.getFile().getMaxBodySize() << std::endl;
			return HttpStatus::e_code::PAYLOAD_TOO_LARGE;
		}
	}
	catch(const std::exception& e) {
		std::cerr << "2. 400 Bad Request - content-length is NAN" << std::endl;
		return HttpStatus::e_code::BAD_REQUEST;
	}

	return HttpStatus::e_code::OK;
}

