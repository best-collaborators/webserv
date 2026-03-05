#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>
#include <iostream>
#include <filesystem>

struct Location
{
	std::filesystem::path				path;
	std::filesystem::path				root;
	bool								autoindex;
	std::string							default_file;
	std::optional<HttpMethodRegistry>	methods_registry;
	HttpPage							return_page;
};

inline std::ostream& operator<<(std::ostream& os, const Location& location)
{
	Logger::displayLog(Logger::e_log_level::INFO, "Location path: " + location.path.string(), "config");

	if (location.methods_registry)
		location.methods_registry->printAllowedMethods();
	if (!location.default_file.empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Index: " + location.default_file, "config");
	if (!location.root.empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Root: " + location.root.string(), "config");

	std::string autoindex_enabled = (location.autoindex ? "true" : "false");
	Logger::displayLog(Logger::e_log_level::INFO, "Autoindex: " + autoindex_enabled, "config");

	return os;
}

inline std::string to_string(const Location& location)
{
	std::string result = "Location path: " + location.path.string() + "\n";
	
	if (location.methods_registry)
		result += "    Methods: allowed\n";
	if (!location.default_file.empty())
		result += "    Index: " + location.default_file + "\n";
	if (!location.root.empty())
		result += "    Root: " + location.root.string() + "\n";
	
	result += "    Autoindex: " + std::string(location.autoindex ? "true" : "false") + "\n";
	
	return result;
}

#endif /* LOCATION_HPP */

