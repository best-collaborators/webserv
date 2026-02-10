#include "RequestParser.hpp"

RequestParser::RequestParser(Request &request, std::string &raw_bits)
: _request(request), _raw_bits(raw_bits) { }

void RequestParser::parse_body()
{
	HttpBodyParser body_parser(_raw_bits, _request);
	body_parser.parse();
}

void RequestParser::parse_headers()
{
	HttpHeaderParser header_parser(_raw_bits, _request);
	header_parser.parse();
}
