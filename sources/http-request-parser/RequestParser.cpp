#include "RequestParser.hpp"

RequestParser::RequestParser(Request &request, std::string &raw_bits) : _parse_context{request, raw_bits} {
}

void RequestParser::parse_body()
{
	HttpBodyParser body_parser(_parse_context);
	body_parser.parse();
}

void RequestParser::parse_headers()
{
	RequestLineValidator line_validator(_parse_context);
	HttpStatus::e_code status = line_validator.validate();

	if (HttpStatus::is_bad(status)) { _parse_context.request.set_status_code(status); return; }

	std::cout << "AFTER REQUEST LINE" << std::endl;
	HttpHeaderParser header_parser(_parse_context);
	status = header_parser.parse();
	if (HttpStatus::is_bad(status)) { _parse_context.request.set_status_code(status); return; }

	_parse_context.request.set_status_code(200);
}
