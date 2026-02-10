#ifndef HEADER_VALIDATOR
#define HEADER_VALIDATOR

#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "ValidatorHelpers.hpp"
#include "Request.hpp"

#include "RequestGenerator.hpp"
#include "RequestLineValidator.hpp"
#include "HttpLimits.hpp"

class HttpHeaderParser
{
private:
	std::string		_buffer;
	std::string		&_raw_bits;
	Request			&_request;

	uint	_validateRequestHeaders();
	int		_contentLengthValidation();
	bool	_isValidHeader();

	HttpHeaderParser() = delete;
	HttpHeaderParser(const HttpHeaderParser && other) = delete;
	HttpHeaderParser(const HttpHeaderParser & other) = delete;

public:
	HttpHeaderParser( std::string &raw_bits, Request &request );
	~HttpHeaderParser() = default;

	void parse();
};

#endif /* HEADER_VALIDATOR */
