#include "ConfigurationFileParser.hpp"

ConfigurationFileParser::ConfigurationFileParser(std::string filename) : _filename(filename)
{
}

ConfigurationFileParser::~ConfigurationFileParser()
{
}



bool ConfigurationFileParser::isConfigFileValid()
{
	std::ifstream ifs(_filename);
	if (ifs.bad()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Cannot access " + _filename, "config-file");
	}

	std::string line;
	while (getline(ifs, line))
	{
		Logger::displayLog(Logger::e_log_level::DEBUG, line, "config-file");

		if (ifs.bad() && !ifs.eof()) {
			Logger::displayLog(Logger::e_log_level::ERROR, "Error happened while reading " + _filename, "config-file");
		}
		else if (ifs.bad() && ifs.eof()) {
			break;
		}

		if (line.empty()) continue;
		if (line == "server:") 
		{
			std::cout << "IN SERVER:" << std::endl;
			while (getline(ifs, line))
			{
				Logger::displayLog(Logger::e_log_level::DEBUG, line, "config-file");

				if (ifs.bad() && !ifs.eof()) {
					Logger::displayLog(Logger::e_log_level::ERROR, "Error happened while reading " + _filename, "config-file");
				}
				else if (ifs.bad() && ifs.eof()) {
					break;
				}

				if (line.empty()) continue;
				if (line == "listen:") 
				{
					
				}
				else if (line == "server_name:")
				{

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
}