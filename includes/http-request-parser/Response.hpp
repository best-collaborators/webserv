#ifndef RESPONSE_GENERATOR_HPP
#define RESPONSE_GENERATOR_HPP

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <chrono>
#include <iomanip>
#include "HttpStatus.hpp"

#include "RequestParseResult.hpp"

class Response
{
private:
	RequestParseResult _parse_result;
	std::streamsize	   _response_length;
	std::string		   _body_content;
	bool			   _is_a_file;
	std::string _root = "data";

	std::string get_file_last_modified_date(const char *filename);
	std::_Put_time<char> get_date_GMT();
	std::string serve_html_webserv_page(std::string errmsg);
	void create_body();
	bool is_set_default_page();
	bool is_fstream_successful(std::fstream &ifs);

public:
	~Response();
	Response();
	std::string form_reponse();
	void set_parse_result(RequestParseResult parse_result);
	uint status_code();
};

#endif /* RESPONSE_GENERATOR_HPP */
