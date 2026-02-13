#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include "HttpBodyParser.hpp"
#include "HttpHeaderParser.hpp"

class RequestParser
{
private:
	ParseContext _parse_context;

	RequestParser(const RequestParser &other) = delete;
	RequestParser(RequestParser &&other) = delete;
	RequestParser & operator=( RequestParser && ) noexcept = delete;

public:
	RequestParser(Request &request, std::string &raw_bits);
	~RequestParser() = default;

	void			parse_body();
	void			parse_headers();
};

#endif /* REQUEST_PARSER_HPP */