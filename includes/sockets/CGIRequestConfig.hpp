#pragma once

#include <regex>

#include "Request.hpp"
#include "CGIConfig.hpp"
#include "RegexMatcher.hpp"

namespace cgi
{
	bool isCGITarget( std::string const & target );
	CGIConfig buildConfig( std::unordered_map<std::string, std::string> const & headers );
}
