#include "ConfigurationFileParser.hpp"

ConfigurationFileParser::ConfigurationFileParser(std::string filename) : _filename(filename)
{
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseServerName(std::string line)
{
	try
	{
		_data._server_name = line.substr(12);
	}
	catch(const std::exception& e)
	{
		Logger::displayLog(Logger::e_log_level::ERROR, "Server name is invalid: " + line, "config");
		return ERROR;
	}
	
	Trimmer::trim(_data._server_name);
	Logger::displayLog(Logger::e_log_level::INFO, "Server name: " + _data._server_name, "config");
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseListen(std::string line)
{
	std::string copy = line;
	std::string ip_addr = RegexMatcher::get_regex_value(copy, HttpRegexPatterns::IP_ADDR_PORT(), 2);
	std::string port = RegexMatcher::get_regex_value(line, HttpRegexPatterns::IP_ADDR_PORT(), 3);

	if (ip_addr.empty() || port.empty()) return ERROR;

	_data._listen_data.ip_address = ip_addr;

	try {
		_data._listen_data.port = std::stoi(port);
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Port is invalid: " + port, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Ip address: " + ip_addr, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Port: " + port, "config");
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::parse()
{
	std::ifstream ifs(_filename);
	if (ifs.bad()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Cannot access " + _filename, "config");
		return e_parse_result::ERROR;
	}

	std::string line;
	while (getline(ifs, line))
	{
		Logger::displayLog(Logger::e_log_level::DEBUG, line, "config");
		if (ifs.bad() && !ifs.eof()) {
			Logger::displayLog(Logger::e_log_level::ERROR, "Error happened while reading " + _filename, "config");
			return e_parse_result::ERROR;
		}
		else if (ifs.bad() && ifs.eof()) {
			break;
		}

		if (line.empty()) continue;
		if (line == "server:") 
		{
			while (getline(ifs, line))
			{
				Logger::displayLog(Logger::e_log_level::DEBUG, line, "config");
		
				if (ifs.bad() && !ifs.eof()) {
					Logger::displayLog(Logger::e_log_level::ERROR, "Error happened while reading " + _filename, "config");
					return ERROR;
				}
				else if (ifs.bad() && ifs.eof()) {
					break;
				}

				if (line.empty()) continue;

				if (line.size() < 1 || *line.begin() != '\t') {
					Logger::displayLog(Logger::e_log_level::ERROR, "Invalid format: " + line, "config");
					return ERROR;
				}
				line.erase(0, 1);

				if (line.compare("listen:") == 0) 
				{
					if (!_data._listen_data.ip_address.empty()) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format listen", "config");
						return ERROR;
					}

					getline(ifs, line);

					if (ifs.bad() && !ifs.eof()) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Error happened while reading " + _filename, "config");
						return ERROR;
					}
					else if (ifs.bad() && ifs.eof()) {
						break;
					}
					if (line.size() < 2 || line.compare(0, 2, "\t\t")) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Invalid format: " + line, "config");
						return ERROR;
					}
					line.erase(0, 2);

					if (_parseListen(line) == ERROR) return ERROR;
				}
				else if (line.compare(0, 12, "server_name:") == 0)
				{
					if (!_data._server_name.empty()) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Invalid format: " + line, "config");
						return ERROR;
					}
					_parseServerName(line);
					
				}
				else if (line == "error_pages:")
				{

				}
				else if (line == "max_body_size:")
				{

				}
				else if (line == "root:")
				{

				}
				else if (line == "index:")
				{

				}
				else if (line == "locations:")
				{
					// iterate locations
				}
				else if (line == "cgi")
				{
					
				}
			}
		}
	}
	return OK;
}