#ifndef PARSE_CONTEXT
#define PARSE_CONTEXT

#include "Request.hpp"
#include <string>

struct ParseContext {
	Request		&request;
	std::string	&raw_bits;
};

#endif /* PARSE_CONTEXT */
