#include "CGIValidator.hpp"

CGIValidator::CGIValidator( ParseContext &parse_context ) : _parse_context(parse_context) { }

CGIValidator::~CGIValidator() { }

HttpStatus::e_code CGIValidator::_validateCGIHeaders( ) {
	bool has_content_type = false;

	headers_map copy = _parse_context.request.move_headers();
	for (auto& [name, value] : copy) {
		if (name == http::headers::CONTENT_TYPE) {
			has_content_type = handleContentType(value);
		}
		else if (name == http::headers::STATUS) {
			auto status_result = handleStatus(value);
			if (status_result != HttpStatus::e_code::OK)
				return status_result;
		}
		else if (name == http::headers::LOCATION) {
			auto loc_result = handleLocation(value);
			if (loc_result != HttpStatus::e_code::OK)
				return loc_result;
		}
	}
	_parse_context.request.set_headers(std::move(copy));

	return has_content_type ? HttpStatus::e_code::OK 
							: HttpStatus::e_code::INTERNAL_SERVER_ERROR;
}

bool CGIValidator::handleContentType(const std::string& value) {
	_parse_context.request.set_header_value(http::headers::CONTENT_TYPE, value);
	return true;
}

HttpStatus::e_code CGIValidator::handleStatus(const std::string& value) {
	try {
		uint status_code = std::stoul(value.substr(0, 3));
		auto code_enum = HttpStatus::code_from_number(status_code);

		if (code_enum == HttpStatus::e_code::UNKNOWN) {
			throw std::logic_error(std::to_string(status_code));
		}

		if (HttpStatus::get_status_code_name(code_enum) != value.substr(4)) {
			Log::warning("CGI reason phrase does not match status code", "cgi");
		}

		_parse_context.request.set_status_code(code_enum);
	}
	catch (const std::exception& e) {
		Log::error("Unknown status code from CGI", "cgi");
		return HttpStatus::e_code::INTERNAL_SERVER_ERROR;
	}
	return HttpStatus::e_code::OK;
}

//! ADD REGEX MATCHER CONST FUNCTIONS
HttpStatus::e_code CGIValidator::handleLocation(const std::string& value) {

	std::string copy = value;
	if (RegexMatcher::get_regex_value(copy, HttpRegexPatterns::FILEPATH()).empty()) {
		Log::error("Invalid location path", "cgi");
		return HttpStatus::e_code::INTERNAL_SERVER_ERROR;
	}

	_parse_context.request.set_header_value(http::headers::LOCATION, value);
	return HttpStatus::e_code::OK;
}

HttpStatus::e_code CGIValidator::_validateCGIOutput( ) noexcept
{
	size_t pos = _parse_context.raw_bits.find("\r\n\r\n");
	if (pos == std::string::npos)
	{
		Log::error("Malformed CGI response: missing header terminator", "cgi");
		return HttpStatus::e_code::INTERNAL_SERVER_ERROR;
	}

	HttpHeaderParser headers_parser(_parse_context);
	headers_parser.parse();

	if (HttpStatus::is_bad(_parse_context.request.get_status_code())) {
		return HttpStatus::e_code::INTERNAL_SERVER_ERROR;
	}
	_parse_context.request.print_http_request_values();
	return _validateCGIHeaders();
}
