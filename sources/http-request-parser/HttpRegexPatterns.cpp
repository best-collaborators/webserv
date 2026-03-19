#include "HttpRegexPatterns.hpp"

const std::regex HttpRegexPatterns::METHOD_PATTERN("(^[A-Z]{1,32}[ ]+)");
const std::regex HttpRegexPatterns::FILEPATH_PATTERN("^( *\\/\\S* *)");
const std::regex HttpRegexPatterns::VERSION_PATTERN("(HTTP\\/(\\d)+.(\\d)+\\s*$)");
const std::regex HttpRegexPatterns::HEADER_PATTERN("(^\\S{1,256}\\: +)");
const std::regex HttpRegexPatterns::CONTENT_DISPOSITION_PATTERN("^[C,c]ontent-[D,d]isposition: form-data;\\s*name=\"(\\S{1,256})\";?\\s*(filename=\"(\\S{1,256})\")?");
const std::regex HttpRegexPatterns::CONTENT_TYPE_PATTERN("^[C,c]ontent-[T,t]ype:\\s*(\\S{1,256}\\/\\S{1,256})\\s*");
const std::regex HttpRegexPatterns::BOUNDARY_PATTERN("^multipart/form-data;\\s*boundary=([^;\\s]+$)");
const std::regex HttpRegexPatterns::HEX_VALUE_PATTERN("^([0-9a-f]+)");
const std::regex HttpRegexPatterns::CGI_VALID_PATH_PATTERN("(^/cgi-bin/\\w+\\.(?:js|py|php|cgi)(?:\\?(?:\\w+=\\w*(?:&\\w+=\\w*)*)?)?$)");
const std::regex HttpRegexPatterns::IP_ADDR_HOST_PATTERN(
	"(^((?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)(?:\\.(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)){3})\\s*:\\s*((?:1000|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5]))\\s*$)"
);
const std::regex HttpRegexPatterns::ERROR_PAGE_PATTERN("^([1-5][0-9][0-9]) *: *(\\s*\\S*)\\s*$");
const std::regex HttpRegexPatterns::NON_WHITESPACE_PATTERN("^ *(\\S+) *$");
const std::regex HttpRegexPatterns::WHITESPACE_PATTERN("^\\s*$");
const std::regex HttpRegexPatterns::LOCATION_PATH_PATTERN("^ *(\\/(?!.*\\.\\.)([A-Za-z0-9._\\-\\/]*))$");
const std::regex HttpRegexPatterns::ALLOWED_METHODS_PATTERN("^ *\" *(GET|POST|DELETE) *(\\| *(GET|POST|DELETE))* *\" *$");
const std::regex HttpRegexPatterns::INDEX_PATTERN("^ *[A-Za-z0-9._-]+ *$");

// Request line patterns
const std::regex& HttpRegexPatterns::METHOD()
{
	return HttpRegexPatterns::METHOD_PATTERN;
}

const std::regex& HttpRegexPatterns::FILEPATH()
{
	return HttpRegexPatterns::FILEPATH_PATTERN;
}

const std::regex& HttpRegexPatterns::VERSION()
{
	return HttpRegexPatterns::VERSION_PATTERN;
}

// Header patterns
const std::regex& HttpRegexPatterns::HEADER()
{
	return HttpRegexPatterns::HEADER_PATTERN;
}

const std::regex& HttpRegexPatterns::CONTENT_DISPOSITION()
{
	return HttpRegexPatterns::CONTENT_DISPOSITION_PATTERN;
}

const std::regex& HttpRegexPatterns::CONTENT_TYPE()
{
	return HttpRegexPatterns::CONTENT_TYPE_PATTERN;
}

const std::regex& HttpRegexPatterns::BOUNDARY()
{
	return HttpRegexPatterns::BOUNDARY_PATTERN;
}

const std::regex& HttpRegexPatterns::HEX_VALUE()
{
	return HttpRegexPatterns::HEX_VALUE_PATTERN;
}

const std::regex &HttpRegexPatterns::CGI_VALID_PATH()
{
	return HttpRegexPatterns::CGI_VALID_PATH_PATTERN;
}

const std::regex& HttpRegexPatterns::IP_ADDR_PORT()
{
	return HttpRegexPatterns::IP_ADDR_HOST_PATTERN;
}

const std::regex& HttpRegexPatterns::GET_ERROR_PAGE()
{
	return HttpRegexPatterns::ERROR_PAGE_PATTERN;
}

const std::regex& HttpRegexPatterns::NON_WHITESPACE()
{
	return HttpRegexPatterns::NON_WHITESPACE_PATTERN;
}

const std::regex& HttpRegexPatterns::WHITESPACE()
{
	return HttpRegexPatterns::WHITESPACE_PATTERN;
}

const std::regex& HttpRegexPatterns::LOCATION_PATH()
{
	return HttpRegexPatterns::LOCATION_PATH_PATTERN;
}

const std::regex& HttpRegexPatterns::ALLOWED_METHODS()
{
	return HttpRegexPatterns::ALLOWED_METHODS_PATTERN;
}

const std::regex& HttpRegexPatterns::INDEX()
{
	return HttpRegexPatterns::INDEX_PATTERN;
}