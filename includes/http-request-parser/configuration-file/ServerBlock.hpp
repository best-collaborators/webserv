#ifndef SERVER_BLOCK_HPP
#define SERVER_BLOCK_HPP

#include "ListenData.hpp"
#include "ErrorPage.hpp"
#include "HttpMethod.hpp"
#include "Location.hpp"
#include "CGI.hpp"
#include "HttpStatus.hpp"
#include <map>
#include <vector>
#include <optional>
#include <string>

struct ServerBlock
{
	std::string									_server_name;
	ListenData									_listen_data;
	std::map<HttpStatus::e_code, std::string>	_error_pages;
	int 										_max_body_size;
	std::filesystem::path						_root;
	std::optional<std::string>					_index;
	std::optional<std::vector<Location>>		_locations;
	std::optional<std::vector<CGIPath>>			_cgi;
	std::bitset<8>								_assigned_fields;
	// std::optional<HttpMethodRegistry>	_default_allowed_methods;
};

std::ostream& operator<<(std::ostream& os, const ServerBlock& block);

#endif /* SERVER_BLOCK_HPP */
