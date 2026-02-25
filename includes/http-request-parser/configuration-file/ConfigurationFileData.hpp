#ifndef CONFIGURATION_FILE_DATA
#define CONFIGURATION_FILE_DATA

#include "ListenData.hpp"
#include "ErrorPage.hpp"
#include "HttpMethod.hpp"
#include "Location.hpp"
#include <vector>
#include <optional>

struct ConfigurationFileData
{
	std::string					_server_name;
	ListenData					_listen_data;
	std::vector<ErrorPage>		_error_pages;
	int 						_max_body_size;
	std::string					_root;
	std::optional<std::string>	_index;
	// ? std::optional<HttpMethod>	_default_allowed_methods;
	std::optional<std::vector<Location>>	_locations;
};

#endif /* CONFIGURATION_FILE_DATA */
