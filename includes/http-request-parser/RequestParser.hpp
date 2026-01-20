#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include <unordered_map>
#include <regex>
#include <string>
#include <string_view> 
#include <cstring>
#include <iostream>
#include <vector>

#include "MultipartFormData.hpp"
#include "MultipartDataValidator.hpp"
#include "Trimmer.hpp"
#include "RequestParseResult.hpp"
#include "RequestGenerator.hpp"
#include "Clock.hpp"

class RequestParser
{
private:
	std::string _buffer;
	std::vector<MultipartFormData> _multipartFormDatas;
	std::unordered_map<std::string, std::string> _http_request_values;
	std::string _request;
	uint _status_code;

	//!Move to a different class
	uint _uploaded_files_count;

	const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
	const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
	const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

	const char *REGEX_HTTP_METHOD = "(^[A-Z]{1,32}[ ]+)";
	const char *REGEX_HTTP_REQUEST_TARGET = "(^/\\S*[\\s]+)";
	const char *REGEX_HTTP_VESRION = "(HTTP\\/(\\d)+.(\\d)+\\s*$)";
	const char *REGEX_HTTP_HEADER = "(^\\S{1,256}:[ ]+)";

	std::string get_regex_value(std::string &line, std::regex regex_method);

	bool is_valid_request_line();
	uint validate_request_line();

	bool add_value_to_map(const char *regex_str, std::string errmsg, std::string key);

	bool is_valid_header();
	int content_length_validation();

	uint validate_request_headers();
	uint validate_request_body();
	
public:
	RequestParser();
	
	void parse_body(std::string request);
	void parse_headers(std::string request);
	void print_http_request_values() const;
	RequestParseResult create_request_parse_result();
};

#endif /* REQUEST_PARSER_HPP */