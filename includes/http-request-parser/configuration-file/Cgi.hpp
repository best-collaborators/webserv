#ifndef CGI
#define CGI

#include <string>
#include "HttpMethodRegistry.hpp"
#include <optional>

struct CGI
{
	//? std::string						path;
	std::string							pass_to;
	std::string							extension;
	std::optional<HttpMethodRegistry>	methods_registry;
};

#endif /* CGI */
