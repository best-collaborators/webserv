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
#include <filesystem>

#include "HttpStatus.hpp"
#include "HttpMessage.hpp"

class Response : HttpMessage
{
private:
	uint					_status_code;
	std::streamsize			_response_length;
	std::string				_root = "data";
	// bool			   _is_a_file;

	std::string				get_file_last_modified_date(const std::string &filename);
	std::_Put_time<char>	get_date_GMT();
	std::string				serve_html_webserv_page(std::string errmsg);
	void					create_body();
	bool					is_set_default_page();
	bool					is_fstream_successful(std::fstream &ifs);

public:
	Response(uint _status_code, std::unordered_map<std::string, std::string> _http_request_values);

	Response() = delete;
	Response(const Response &other) = default;
	Response(Response &&other) = default;
	Response & operator=( Response && ) noexcept = default;
	~Response();

	std::string form_response();
	uint status_code();
};

#endif /* RESPONSE_GENERATOR_HPP */
