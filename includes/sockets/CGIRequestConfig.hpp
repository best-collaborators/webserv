#pragma once

#include <regex>
#include <string_view>

#include "Request.hpp"
#include "CGIConfig.hpp"
#include "ServerBlock.hpp"
#include "RegexMatcher.hpp"

namespace cgi
{
	CGIConfig buildConfig( std::unordered_map<std::string, std::string> const & headers, File const & file );
}
