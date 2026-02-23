#ifndef LOCATION
#define LOCATION

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>

struct Location
{
	std::string							path;
	std::string							root;
	bool								autoindex;
	std::optional<std::string>			default_file;
	std::optional<HttpMethodRegistry>	methods_registry;
};

#endif /* LOCATION */
