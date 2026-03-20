#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <ostream>
#include <optional>
#include <filesystem>
#include <unordered_set>

#include "HttpMethodRegistry.hpp"

struct CGIPath
{
	std::string							pass_to;
	std::unordered_set<std::string>		extensions;
	std::filesystem::path				path = "/";
	size_t								max_body_size;
	std::optional<HttpMethodRegistry>	methods_registry;
};

std::ostream& operator<<(std::ostream& os, const CGIPath& cgi);
std::string to_string(std::unordered_set<std::string> extensions);
std::string to_string(const CGIPath& cgi);

#endif /* CGIPath_HPP */

