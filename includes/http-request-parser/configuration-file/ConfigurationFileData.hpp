#ifndef CONFIGURATION_FILE_DATA_HPP
#define CONFIGURATION_FILE_DATA_HPP

#include "ListenData.hpp"
#include "ErrorPage.hpp"
#include "HttpMethod.hpp"
#include "Location.hpp"
#include "CGI.hpp"
#include <vector>
#include <optional>

struct ConfigurationFileData
{
	std::string								_server_name;
	ListenData								_listen_data;
	std::vector<HttpPage>					_error_pages;
	int 									_max_body_size;
	std::filesystem::path					_root;
	std::optional<std::string>				_index;
	std::optional<std::vector<Location>>	_locations;
	std::optional<std::vector<CGIPath>>			_cgi;
	// std::optional<HttpMethodRegistry>	_default_allowed_methods;
};

#endif /* CONFIGURATION_FILE_DATA_HPP */
