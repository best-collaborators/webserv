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
	RequestLineValidator line_validator(_raw_bits, _request);
	HttpStatus::e_code status = line_validator.validate();

	if (HttpStatus::is_bad(status)) { _request.set_status_code(status); return; }

	std::cout << "AFTER REQUEST LINE" << std::endl;
	HttpHeaderParser header_parser(_raw_bits, _request);
	status = header_parser.parse();
	if (HttpStatus::is_bad(status)) { _request.set_status_code(status); return; }

	_request.set_status_code(200);
}
