#ifndef REQUEST_LINE_VALIDATOR
#define REQUEST_LINE_VALIDATOR

#include <iostream>

#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "Trimmer.hpp"
#include "Request.hpp"
#include "RequestStringUtils.hpp"
#include "PercentEncoder.hpp"
#include "HttpLimits.hpp"

#include "HttpStatus.hpp"
#include "HttpMethod.hpp"

class RequestLineValidator
{
private:
	const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
	const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
	const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

	std::string		_buffer;
	std::string		&_raw_bits;
	Request			&_request;

	//Add to separate utils:
	bool add_value_to_map( std::regex regex_str, std::string errmsg, std::string key );

	RequestLineValidator() = delete;
	RequestLineValidator(const RequestLineValidator & other) = delete;
	RequestLineValidator(const RequestLineValidator && other) = delete;

public:
	RequestLineValidator( std::string &raw_bits, Request &request );
	~RequestLineValidator() = default;

	bool				is_valid_request_line();
	HttpStatus::e_code	validate();
};

#endif /* REQUEST_LINE_VALIDATOR */
