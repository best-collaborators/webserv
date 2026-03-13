#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>
#include <unordered_map>

struct CGIPath
{
	// ? bool										autoindex;
	std::string										path;
	std::unordered_map<std::string, std::string>	pass_to;
	std::string										extension;
	std::string										index;
	std::optional<HttpMethodRegistry>				methods_registry;
};

inline std::ostream& operator<<(std::ostream& os, const CGIPath& cgi)
{
	if (cgi.methods_registry.has_value())
		cgi.methods_registry->printAllowedMethods();
	Logger::displayLog(Logger::e_log_level::INFO, "Path: " + cgi.path, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Index: " + cgi.index, "config");
	for (const auto &[name, value] : cgi.pass_to) {
		Logger::displayLog(Logger::e_log_level::INFO, "Extension: " + name + " Executable: " + value, "config");
	}
	Logger::displayLog(Logger::e_log_level::INFO, "Extension: " + cgi.extension, "config");

	return os;
}

inline std::string to_string(const CGIPath& cgi)
{
	std::string result = "";
	result += "    path: " + cgi.path + "\n";
	result += "    extension: " + cgi.extension + "\n";
	result += "    index: " + cgi.index + "\n";
	result += "    pass_to: {";
	for (const auto& [name, value] : cgi.pass_to) {
		result += "\n    " + name + ": " + value;
	}
	result += "\n  }\n";
	if (cgi.methods_registry.has_value())
		result += "    methods_registry: " + cgi.methods_registry.value().to_string() + "\n";
	return result;
}

#endif /* CGI_HPP */

