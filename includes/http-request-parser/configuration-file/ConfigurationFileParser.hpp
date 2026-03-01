#ifndef CONFIGURATION_FILE_PARSER_HPP
#define CONFIGURATION_FILE_PARSER_HPP

#include <iostream>
#include <fstream>
#include "Logger.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "ServerBlock.hpp"
#include "Trimmer.hpp"

class ConfigurationFileParser
{
public:
	enum e_parse_result 
	{
		OK,
		ERROR
	};

	ConfigurationFileParser(std::string filename, std::vector<ServerBlock> &server_blocks);
	ConfigurationFileParser() = delete;
	~ConfigurationFileParser() = default;

	e_parse_result parse();
	const ServerBlock& getData() const;

private:
	std::string				_filename;
	std::vector<ServerBlock> &_server_blocks;
	ServerBlock				_current_server_block;

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
	void					_clearServerBlock();
};

#endif /* CONFIGURATION_FILE_PARSER_HPP */
