#ifndef CONFIGURATION_FILE_PARSER_HPP
#define CONFIGURATION_FILE_PARSER_HPP

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unistd.h>
#include "Logger.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"
#include "ServerBlock.hpp"
#include "Trimmer.hpp"

class ConfigurationFileParser
{
public:
	using server_block_map = std::unordered_map<ListenData, ServerBlock, ListenDataHash>;

	enum e_parse_result 
	{
		OK,
		ERROR
	};

	ConfigurationFileParser(std::string filename, server_block_map &server_blocks);
	ConfigurationFileParser() = delete;
	~ConfigurationFileParser() = default;

	e_parse_result parse();
	const ServerBlock& getData() const;

private:
	std::string				_filename;
	ServerBlock				_current_server_block;
	server_block_map		&_server_blocks;

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

	e_parse_result			_parseServerBlock();
	e_parse_result			_parseListenDirective();

	// Parsing orchestration
    e_parse_result _parseAllServerBlocks(std::ifstream &ifs);
    e_parse_result _parseSingleServerBlock(std::ifstream &ifs, std::string &line, bool &extra_line);
    e_parse_result _dispatchServerDirective(std::ifstream &ifs, std::string &line, bool &extra_line);

    // Directive handlers
    e_parse_result _checkDuplicateField(size_t bit_index, const std::string &field_name, std::bitset<8UL> &assigned_fields);
    e_parse_result _handleListenDirective(std::ifstream &ifs, std::string &line, bool &extra_line);
    e_parse_result _handleServerNameDirective(std::string &line, bool &extra_line);
    e_parse_result _handleErrorPagesDirective(std::ifstream &ifs, std::string &line, bool &extra_line);
    e_parse_result _handleMaxBodySizeDirective(std::string &line, bool &extra_line);
    e_parse_result _handleRootDirective(std::string &line, bool &extra_line);
    e_parse_result _handleIndexDirective(std::string &line, bool &extra_line);
    e_parse_result _handleLocationsDirective(std::ifstream &ifs, std::string &line, bool &extra_line);
    e_parse_result _handleCGIDirective(std::ifstream &ifs, std::string &line, bool &extra_line);

    // Validation
    e_parse_result _validateServerBlocks();
    e_parse_result _validateRequiredFields(const ServerBlock &s_block);
    e_parse_result _validateIndexPath(const ServerBlock &s_block);
    e_parse_result _validateErrorPages(ServerBlock &s_block);
    e_parse_result _validateLocations(ServerBlock &s_block);

	std::string _extractDirectiveValue(std::string &line, size_t keyword_length);

	// Location directive sub-parsers
	e_parse_result _parseAllowedMethods(std::string &line, std::optional<HttpMethodRegistry> &registry, std::bitset<8> &fields);

	e_parse_result _dispatchLocationDirective(std::string &line, Location &location, std::bitset<8> &fields);
	e_parse_result _parseLocationIndex(std::string &line, Location &location, std::bitset<8> &fields);
	e_parse_result _parseLocationRoot(std::string &line, Location &location, std::bitset<8> &fields);
	e_parse_result _parseLocationRedirect(std::string &line, Location &location, std::bitset<8> &fields);
	e_parse_result _parseLocationAutoindex(std::string &line, Location &location, std::bitset<8> &fields);

	// CGI directive sub-parsers
	e_parse_result _dispatchCGIDirective(std::string &line, CGIPath &cgi, std::bitset<8> &fields);
	e_parse_result _parseCGIPassTo(std::string &line, CGIPath &cgi, std::bitset<8> &fields);
	e_parse_result _parseCGIIndex(std::string &line, CGIPath &cgi, std::bitset<8> &fields);
};

#endif /* CONFIGURATION_FILE_PARSER_HPP */
