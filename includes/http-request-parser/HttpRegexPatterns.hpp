#ifndef HTTP_REGEX_PATTERNS_HPP
#define HTTP_REGEX_PATTERNS_HPP

#include <regex>

class HttpRegexPatterns {
public:
	// Request line patterns
	static const std::regex& METHOD();
	static const std::regex& REQUEST_TARGET();
	static const std::regex& VERSION();

	// Header patterns
	static const std::regex& HEADER();
	static const std::regex& CONTENT_DISPOSITION();
	static const std::regex& CONTENT_TYPE();
	static const std::regex& BOUNDARY();

	// Encoding patterns
	static const std::regex& PERCENT_ENCODING();
	static const std::regex& HEX_VALUE();

private:
	HttpRegexPatterns() = delete;

};

#endif /* HTTP_REGEX_PATTERNS_HPP */