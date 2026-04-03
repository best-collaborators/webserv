#include "ServerBlock.hpp"

std::string to_string(const ServerBlock& block)
{
	std::string result = "ServerBlock {\n";
	result += "  listen_data: " + block._listen_data.ip_address + ":" + std::to_string(block._listen_data.port) + "\n";
	result += "  server_name: " + block._server_name + "\n";
	result += "  root: " + block._root.string() + "\n";
	
	if (block._index.has_value())
		result += "  index: " + block._index.value() + "\n";
	
	result += "  error_pages:\n";
	for (const auto& [code, page] : block._error_pages)
		result += "    " + std::to_string(HttpStatus::number_from_code(code)) + ": " + page + "\n";
	
	if (block._locations.has_value())
	{
		result += "  locations:\n";
		for (const auto& loc : block._locations.value())
			result += "    " + loc.to_string() + "\n";
	}
	
	if (block._cgi.has_value())
	{
		result += "  cgi:\n";
		for (const auto& cgi : block._cgi.value())
			result += to_string(cgi) + "\n";
	}
	
	result += "}\n";
	return result;
}

std::ostream& operator<<(std::ostream& os, const ServerBlock& block)
{
	os << to_string(block);
	return os;
}
