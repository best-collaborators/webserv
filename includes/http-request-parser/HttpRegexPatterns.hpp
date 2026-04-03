#ifndef HTTP_REGEX_PATTERNS_HPP
#define HTTP_REGEX_PATTERNS_HPP

#include <regex>

class HttpRegexPatterns {
public:
	// Request line patterns
	static const std::regex& METHOD();
	static const std::regex& FILEPATH();

	// Header patterns
	static const std::regex& CONTENT_DISPOSITION();
	static const std::regex& CONTENT_TYPE();
	static const std::regex& BOUNDARY();

	// Encoding patterns
	static const std::regex& HEX_VALUE();

	// CGI patterns
	static const std::regex& CGI_VALID_PATH();

	// Configuration file patterns
	static const std::regex& GET_ERROR_PAGE();
	static const std::regex& IP_ADDR_PORT();
	static const std::regex& NON_WHITESPACE();
	static const std::regex& WHITESPACE();
	static const std::regex& LOCATION_PATH();
	static const std::regex& ALLOWED_METHODS();
	static const std::regex& INDEX();

private:
	HttpRegexPatterns() = delete;

	// Private regex pattern attributes
	static const std::regex METHOD_PATTERN;
	static const std::regex FILEPATH_PATTERN;
	static const std::regex VERSION_PATTERN;
	static const std::regex HEADER_PATTERN;
	static const std::regex CONTENT_DISPOSITION_PATTERN;
	static const std::regex CONTENT_TYPE_PATTERN;
	static const std::regex BOUNDARY_PATTERN;
	static const std::regex HEX_VALUE_PATTERN;
	static const std::regex CGI_VALID_PATH_PATTERN;
	static const std::regex IP_ADDR_HOST_PATTERN;
	static const std::regex ERROR_PAGE_PATTERN;
	static const std::regex NON_WHITESPACE_PATTERN;
	static const std::regex WHITESPACE_PATTERN;
	static const std::regex LOCATION_PATH_PATTERN;
	static const std::regex ALLOWED_METHODS_PATTERN;
	static const std::regex INDEX_PATTERN;
};


#endif /* HTTP_REGEX_PATTERNS_HPP */