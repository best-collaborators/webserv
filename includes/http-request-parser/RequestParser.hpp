#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include <unordered_map>

#include <regex>

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>

#include <string_view> 

#include "MultipartFormData.hpp"
#include "MultipartDataValidator.hpp"
#include "RequestGenerator.hpp"
#include "Trimmer.hpp"

class RequestParser {

	public:
		uint get_status_code();
		RequestParser(std::unordered_map<std::string, std::string> &http_request_values, std::string request);

	private:
		std::string _buffer;
		std::vector <MultipartFormData> _multipartFormDatas;
		std::unordered_map<std::string, std::string> &_http_request_values;
		std::string _request;

		const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
		const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
		const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

		const char *REGEX_HTTP_METHOD = "(^[A-Z]{1,32}[ ]+)";
		const char *REGEX_HTTP_REQUEST_TARGET = "(^\\/\\S+[ ]+)";
		const char *REGEX_HTTP_VESRION = "(HTTP\\/(\\d)+.(\\d)+\\s*$)";

		std::string get_regex_value(std::string &line, std::regex regex_method);

		bool is_valid_request_line();
		uint validate_request_line();

		bool add_value_to_map(const char *regex_str, std::string errmsg, std::string key);

		bool is_valid_header();
		int content_length_validation();

		uint validate_request_headers();
		uint validate_request_body();

		int parse_multipart_form();


		std::string cut_after_new_line(std::string &line);
};

#endif /* REQUEST_PARSER_HPP */