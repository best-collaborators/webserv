#include "HttpRegexPatterns.hpp"

	// const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
	// const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
	// const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

	// const char *REGEX_HTTP_METHOD =
	// const char *REGEX_HTTP_REQUEST_TARGET = "";
	// const char *REGEX_HTTP_VESRION = "";
	// const char *REGEX_HTTP_HEADER = "";

// Request line patterns
static const std::regex& METHOD()
{
	return std::regex("(^[A-Z]{1,32}[ ]+)");
}

static const std::regex& REQUEST_TARGET()
{
	return std::regex("^(/[\\S]*)");
}

static const std::regex& VERSION()
{
	return std::regex("(HTTP\\/(\\d)+.(\\d)+\\s*$)");
}


// Header patterns
static const std::regex& HEADER()
{
	return std::regex("(^\\S{1,256}:[ ]+)");
}

static const std::regex& CONTENT_DISPOSITION()
{
	
}

static const std::regex& CONTENT_TYPE()
{
	
}

static const std::regex& BOUNDARY()
{
	
}


// Encoding patterns
static const std::regex& PERCENT_ENCODING()
{

}

static const std::regex& HEX_VALUE()
{

}

