#ifndef HEADER_VALIDATOR
#define HEADER_VALIDATOR

#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "RequestStringUtils.hpp"

#include "RequestGenerator.hpp"
#include "RequestLineValidator.hpp"
#include "HttpLimits.hpp"
#include "HttpHeaders.hpp"

#include "HttpStatus.hpp"
#include "ParseContext.hpp"

#include "IParser.hpp"

class HttpHeaderParser : public IParser
{
private:
	ParseContext	&_parse_context;

	HttpStatus::e_code  _validateRequestHeaders();
	HttpStatus::e_code  _contentLengthValidation();
	bool				_isValidHeader(std::string &buffer);

	bool				_isValidHeaderLength(const std::string &buffer) const;
	void 				_normalizeHeader(std::string &name, std::string &value) const;
	bool				_isCriticalHeader(const std::string &name) const;
	bool				_validateSpecialHeaders(const std::string &name,
											const std::string &value,
											const std::string &buffer) const;
	bool				_handleDuplicates(const std::string &name,
											const std::string &buffer);
	std::pair<std::string, std::string> _parseHeader(const std::string &buffer) const;

	HttpHeaderParser() = delete;
	HttpHeaderParser(const HttpHeaderParser && other) = delete;
	HttpHeaderParser(const HttpHeaderParser & other) = delete;

public:
	HttpHeaderParser( ParseContext &parse_context );
	~HttpHeaderParser() = default;

	void parse();
};

#endif /* HEADER_VALIDATOR */
