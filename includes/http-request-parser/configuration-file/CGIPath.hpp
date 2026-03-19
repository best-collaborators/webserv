#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>
#include <unordered_set>
#include <unordered_map>

struct CGIPath
{
	std::string							pass_to;
	std::unordered_set<std::string>		extensions;
	std::filesystem::path				path = "/";
	size_t								max_body_size;
	std::optional<HttpMethodRegistry>	methods_registry;
};

inline std::ostream& operator<<(std::ostream& os, const CGIPath& cgi)
{
	if (cgi.methods_registry.has_value())
		cgi.methods_registry->printAllowedMethods();
	Logger::displayLog(Logger::e_log_level::INFO, "Pass to: " + cgi.pass_to, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Extensions:", "config");
	for (const auto &name : cgi.extensions) {
		Logger::displayLog(Logger::e_log_level::INFO, "   "  + name, "config");
	}
	return os;
}

inline std::string to_string(std::unordered_set<std::string> extensions)
{
	if (extensions.empty()) return "";

	std::string result = "";
	result += "    Extensions: {";
	for (const auto& name : extensions) {
		result += "\n    " + name;
	}
	result += "\n  }\n";
	return result;
}

inline std::string to_string(const CGIPath& cgi)
{
	std::string result = "";
	result += "    Pass_to: " + cgi.pass_to + "\n";
	result += "    Extensions: {";
	for (const auto& name : cgi.extensions) {
		result += "\n    " + name;
	}
	result += "\n  }\n";
	if (cgi.methods_registry.has_value())
		result += "    methods_registry: " + cgi.methods_registry.value().to_string() + "\n";
	return result;
}

#endif /* CGIPath_HPP */

