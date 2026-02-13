#ifndef REQUEST_LINE_VALIDATOR
#define REQUEST_LINE_VALIDATOR

#include <iostream>

#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "Trimmer.hpp"
#include "RequestStringUtils.hpp"
#include "PercentEncoder.hpp"
#include "HttpLimits.hpp"

#include "HttpStatus.hpp"
#include "HttpMethod.hpp"
#include "ParseContext.hpp"

class RequestLineValidator
{
private:
	const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
	const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
	const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

	ParseContext &_parse_context;

	//Add to separate utils:
	
	bool _addValueToMap( std::regex regex_str, std::string &buffer, const char *errmsg, std::string key );

	RequestLineValidator() = delete;
	RequestLineValidator(const RequestLineValidator & other) = delete;
	RequestLineValidator(const RequestLineValidator && other) = delete;

	bool				_isValidRequestLine(std::string &buffer);

public:
	RequestLineValidator( ParseContext &parse_context );
	~RequestLineValidator() = default;

	HttpStatus::e_code	validate();
};

#endif /* REQUEST_LINE_VALIDATOR */
