#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>

struct CGIPath
{
	// ? bool							autoindex;
	std::string							path;
	std::string							pass_to;
	std::string							extension;
	std::optional<HttpMethodRegistry>	methods_registry;
};

inline std::ostream& operator<<(std::ostream& os, const CGIPath& cgi)
{
	if (cgi.methods_registry.has_value())
		cgi.methods_registry->printAllowedMethods();
	Logger::displayLog(Logger::e_log_level::INFO, "Path: " + cgi.path, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Pass_to: " + cgi.pass_to, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Extension: " + cgi.extension, "config");

	return os;
}

#endif /* CGI_HPP */

