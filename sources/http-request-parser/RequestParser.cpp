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
	HttpMethod::initAllowedMethods();

	RequestLineValidator line_validator(_parse_context);
	line_validator.parse();

	if (_parse_context.request.isStatusCodeBad()) { return; }

	HttpHeaderParser header_parser(_parse_context);
	header_parser.parse();
	if (_parse_context.request.isStatusCodeBad()) { return; }
}
