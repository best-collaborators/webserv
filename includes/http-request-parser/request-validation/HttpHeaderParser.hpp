#ifndef HEADER_VALIDATOR
#define HEADER_VALIDATOR

#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "RequestStringUtils.hpp"

#include "RequestGenerator.hpp"
#include "RequestLineValidator.hpp"
#include "HttpLimits.hpp"

#include "HttpStatus.hpp"
#include "ParseContext.hpp"

#include "IParser.hpp"

class HttpHeaderParser : IParser
{
private:
	ParseContext	&_parse_context;

	HttpStatus::e_code  _validateRequestHeaders();
	HttpStatus::e_code  _contentLengthValidation();
	bool				_isValidHeader(std::string &buffer);

	HttpHeaderParser() = delete;
	HttpHeaderParser(const HttpHeaderParser && other) = delete;
	HttpHeaderParser(const HttpHeaderParser & other) = delete;

public:
	HttpHeaderParser( ParseContext &parse_context );
	~HttpHeaderParser() = default;

	void parse();
};

#endif /* HEADER_VALIDATOR */
