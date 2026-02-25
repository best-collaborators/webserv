#ifndef CONFIGURATION_FILE_PARSER
#define CONFIGURATION_FILE_PARSER

#include <iostream>
#include <fstream>
#include "Logger.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "ConfigurationFileData.hpp"
#include "Trimmer.hpp"

class ConfigurationFileParser
{
public:
	enum e_parse_result 
	{
		OK,
		ERROR
	};

	ConfigurationFileParser(std::string filename);
	ConfigurationFileParser() = delete;
	~ConfigurationFileParser() = default;

	e_parse_result parse();
	const ConfigurationFileData& getData() const;

private:
	ConfigurationFileData _data;
	std::string _filename;
	e_parse_result		_parseListen(std::string line);
	e_parse_result		_parseServerName(std::string line);
};

#endif /* CONFIGURATION_FILE_PARSER */
