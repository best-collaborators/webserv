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
#include "HttpHeaders.hpp"
#include "HttpMethodRegistry.hpp"
#include "ParseContext.hpp"

#include "IParser.hpp"

class RequestLineValidator : public IParser
{
public:
	enum e_parse_result {
		NO_FILE_IN_CONFIG,
		MATCH_FOUND,
		RELOCATION,
		UNKNOWN_ERROR
	};

	RequestLineValidator( ParseContext &parse_context );
	~RequestLineValidator() = default;

	void	parse();

private:
	const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
	const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
	const char *ERROR_HTTP_VERSION = "LOG: ERROR INVALID REQUEST VERSION";

	ParseContext &_parse_context;

	//Add to separate utils:

	bool _addValueToMap( std::regex regex_str, std::string &buffer, const char *errmsg, std::string key );

	RequestLineValidator() = delete;
	RequestLineValidator(const RequestLineValidator & other) = delete;
	RequestLineValidator(const RequestLineValidator && other) = delete;

	bool _isValidRequestLine(std::string &buffer);
	bool _isValidHttpVersion();
	bool _isValidUriLength();
	bool _isMethodAllowed();
	bool _isCGIPathValid();
	e_parse_result _isRequestTargetInConfigFile(std::string &request_target);
};

#endif /* REQUEST_LINE_VALIDATOR */
