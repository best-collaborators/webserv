#ifndef CONFIGURATION_FILE_PARSER_HPP
#define CONFIGURATION_FILE_PARSER_HPP

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
	std::bitset<8>			_assigned_fields;

	ConfigurationFileData	_data;
	std::string				_filename;
	
	e_parse_result			_parseListen(std::string &line);
	e_parse_result			_parseServerName(std::string &line);
	e_parse_result			_parseErrorPages(std::string &line);
	e_parse_result			_parseMaxBodySize(std::string &line);
	e_parse_result			_parseRoot(std::string &line);
	e_parse_result			_parseIndex(std::string &line);
	e_parse_result			_parseLocations(std::ifstream &ifs, std::string &line);
	e_parse_result			_parseCGI(std::ifstream &ifs, std::string &line);
	bool					_isStreamGood(std::ifstream &ifs);
	bool					_isStreamFinished(std::ifstream &ifs);
	bool					_validateAndConsumeIndent(std::string &line, size_t intend_level, char c, bool show_msg = true);
	void					_updateAllowedMethods(std::string &method_str, HttpMethodRegistry &methods_registry);
};

#endif /* CONFIGURATION_FILE_PARSER_HPP */
