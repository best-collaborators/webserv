#ifndef CONFIGURATION_FILE_PARSER
#define CONFIGURATION_FILE_PARSER

#include "ListenData.hpp"
#include "ErrorPage.hpp"
#include "HttpMethod.hpp"
#include "Location.hpp"
#include <vector>
#include <optional>

class ConfigurationFileData
{
private:
	std::string					_server_name;
	ListenData					_listen_data;
	std::vector<ErrorPage>		_error_pages;
	int 						_max_body_size;
	std::string					_root;
	std::optional<std::string>	_index;
	// ? std::optional<HttpMethod>	_default_allowed_methods;
	std::optional<std::vector<Location>>	_locations;
	

public:
	ConfigurationFileData(/* args */);
	~ConfigurationFileData();
};

#endif /* CONFIGURATION_FILE_PARSER */
