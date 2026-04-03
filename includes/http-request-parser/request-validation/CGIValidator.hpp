#ifndef CGI_VALIDATOR_HPP
#define CGI_VALIDATOR_HPP

#include "HttpStatus.hpp"
#include "HttpHeaders.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "ParseContext.hpp"
#include "HttpHeaderParser.hpp"
#include "RegexMatcher.hpp"
#include <unordered_map>

class CGIValidator
{
private:
	using headers_map = std::unordered_map<std::string, std::string>;
	ParseContext _parse_context;

	bool handleContentType(const std::string& value);
	HttpStatus::e_code handleStatus(const std::string& value);
	HttpStatus::e_code handleLocation(const std::string& value);

public:
	CGIValidator( ParseContext &parse_context );
	CGIValidator(/* args */) = delete;
	~CGIValidator();

	CGIValidator(const CGIValidator && other) = delete;
	CGIValidator(const CGIValidator & other) = delete;

	HttpStatus::e_code _validateCGIHeaders( );
	HttpStatus::e_code _validateCGIOutput( ) noexcept;
};




#endif /* CGI_VALIDATOR_HPP */