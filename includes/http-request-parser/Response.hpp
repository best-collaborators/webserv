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

#include "HttpResponse.hpp"

class Response
{
private:
	HttpResponse _reponse;
	std::streamsize	   _response_length;
	std::string		   _body_content;
	// bool			   _is_a_file;
	std::string _root = "data";

	std::string get_file_last_modified_date(const std::string *filename);
	std::_Put_time<char> get_date_GMT();
	std::string serve_html_webserv_page(std::string errmsg);
	void create_body();
	bool is_set_default_page();
	bool is_fstream_successful(std::fstream &ifs);

public:
	~Response();
	Response() = delete;
	Response(Response &other) = default;
	Response(uint _status_code, std::unordered_map<std::string, std::string> _http_request_values);
	std::string form_reponse();
	uint status_code();
};

#endif /* RESPONSE_GENERATOR_HPP */
