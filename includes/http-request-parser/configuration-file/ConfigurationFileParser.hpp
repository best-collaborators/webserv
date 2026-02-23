#ifndef CONFIGURATION_FILE_PARSER
#define CONFIGURATION_FILE_PARSER

#include <iostream>
#include <fstream>
#include "Logger.hpp"

class ConfigurationFileParser
{
private:
	std::string _filename;
public:
	ConfigurationFileParser(std::string filename);
	ConfigurationFileParser() = delete;
	~ConfigurationFileParser();

	bool isConfigFileValid();
};

#endif /* CONFIGURATION_FILE_PARSER */
