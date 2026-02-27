#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>
#include <iostream>

struct Location
{
	std::string							path;
	std::string							root;
	bool								autoindex;
	std::string							default_file;
	std::optional<HttpMethodRegistry>	methods_registry;
	HttpPage							return_page;
};

inline std::ostream& operator<<(std::ostream& os, const Location& location)
{
	Logger::displayLog(Logger::e_log_level::INFO, "Location path: " + location.path, "config");

	if (location.methods_registry)
		location.methods_registry->printAllowedMethods();
	if (!location.default_file.empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Index: " + location.default_file, "config");
	if (!location.root.empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Root: " + location.root, "config");

	std::string autoindex_enabled = (location.autoindex ? "true" : "false");
	Logger::displayLog(Logger::e_log_level::INFO, "Autoindex: " + autoindex_enabled, "config");

	return os;
}

#endif /* LOCATION_HPP */

