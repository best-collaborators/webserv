#include "HttpRegexPatterns.hpp"

const std::regex HttpRegexPatterns::METHOD_PATTERN("(^[A-Z]{1,32}[ ]+)");
const std::regex HttpRegexPatterns::REQUEST_TARGET_PATTERN("^(/[\\S]*)");
const std::regex HttpRegexPatterns::VERSION_PATTERN("(HTTP\\/(\\d)+.(\\d)+\\s*$)");
const std::regex HttpRegexPatterns::HEADER_PATTERN("(^\\S{1,256}:[ ]+)");
const std::regex HttpRegexPatterns::CONTENT_DISPOSITION_PATTERN("^[C,c]ontent-[D,d]isposition: form-data;\\s*name=\"(\\S{1,256})\";?\\s*(filename=\"(\\S{1,256})\")?");
const std::regex HttpRegexPatterns::CONTENT_TYPE_PATTERN("^[C,c]ontent-[T,t]ype:\\s*(\\S{1,256}\\/\\S{1,256})\\s*");
const std::regex HttpRegexPatterns::BOUNDARY_PATTERN("^multipart/form-data;\\s*boundary=([^;\\s]+$)");
const std::regex HttpRegexPatterns::HEX_VALUE_PATTERN("^([0-9a-f]+)");
const std::regex HttpRegexPatterns::CGI_VALID_PATH_PATTERN("(^/cgi-bin/\\w+\\.(?:js|py|php|cgi)(?:\\?(?:\\w+=\\w*(?:&\\w+=\\w*)*)?)?$)");

// Request line patterns
const std::regex& HttpRegexPatterns::METHOD()
{
	return HttpRegexPatterns::METHOD_PATTERN;
}

const std::regex& HttpRegexPatterns::REQUEST_TARGET()
{
	return HttpRegexPatterns::REQUEST_TARGET_PATTERN;
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
