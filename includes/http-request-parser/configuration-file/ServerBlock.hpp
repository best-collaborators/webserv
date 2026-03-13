#ifndef SERVER_BLOCK_HPP
#define SERVER_BLOCK_HPP

#include "ListenData.hpp"
#include "HttpPage.hpp"
#include "HttpMethod.hpp"
#include "Location.hpp"
#include "CGI.hpp"
#include "HttpStatus.hpp"
#include <unordered_map>
#include <vector>
#include <optional>
#include <string>

using error_map = std::unordered_map<HttpStatus::e_code, std::string>;

struct ServerBlock
{
	std::string							  _server_name;
	ListenData							  _listen_data;
	error_map							  _error_pages;
	size_t								  _max_body_size;
	std::filesystem::path				  _root;
	std::optional<std::string>			  _index;
	std::optional<std::vector<Location>>  _locations;
	std::optional<std::vector<CGIPath>>	  _cgi;
	std::bitset<8>						  _assigned_fields;
	Location							  _root_restrictions;
};

std::ostream& operator<<(std::ostream& os, const ServerBlock& block);

#endif /* SERVER_BLOCK_HPP */
