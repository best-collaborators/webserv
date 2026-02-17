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
#include "HttpHeaders.hpp"
#include "HttpMessage.hpp"

class Response : HttpMessage
{
private:
	HttpStatus::e_code		_status_code;
	std::size_t				_response_length;
	std::streampos			_content_length;
	std::string				_root = "data";
	size_t					_bytes_sent;
	ssize_t					_bytes_read;
	size_t					_buffer = 10240;
	bool					_is_default_page;
	std::string				_header_str;

	std::string				get_file_last_modified_date(const std::string &filename);
	std::_Put_time<char>	get_date_GMT();
	std::string				serve_html_webserv_page(std::string errmsg);
	void					is_set_default_page();
	bool					is_ifstream_successful(std::ifstream &ifs);
	void					set_content_type(std::string filename);
	std::streampos			get_file_size();
	std::streampos			get_file_read_position();

public:
	Response() = default;
	Response(const Response &other) = default;
	Response(Response &&other) = default;
	Response & operator=( Response && ) noexcept = default;
	~Response() = default;

	// Response(uint status_code, std::unordered_map<std::string, std::string> http_request_values);

	std::string form_response(HttpStatus::e_code _status_code, std::unordered_map<std::string, std::string> &&_http_request_values, std::string body = "");
	HttpStatus::e_code status_code();
	size_t get_total_response_length();
	size_t get_current_length();
	// void set_response_length(size_t response_length);
	void consume_body(size_t consume_length);
	void					read_body_partially();
	std::string &get_body();
};

#endif /* RESPONSE_GENERATOR_HPP */
