#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include <unordered_map>

#include <regex>

#include <string>
#include <cstring>
#include <iostream>
#include <fstream>

#include "MultipartFormData.hpp"
#include "RequestGenerator.hpp"
#include "Trimmer.hpp"

class RequestParser {

	public:
		uint get_status_code(std::string request);
		RequestParser(std::unordered_map<std::string, std::string> &http_request_values);

	private:
		std::vector <MultipartFormData> multipartFormDatas;
		std::string buffer;
		std::unordered_map<std::string, std::string> &_http_request_values;

		const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
		const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
		const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

		const char *REGEX_HTTP_METHOD = "(^[A-Z]{1,32}[ ]+)";
		const char *REGEX_HTTP_REQUEST_TARGET = "(^\\/\\S+[ ]+)";
		const char *REGEX_HTTP_VESRION = "(HTTP\\/(\\d)+.(\\d)+\\s*$)";

		std::string get_regex_value(std::string &line, std::regex regex_method);

		bool is_valid_request_line();
		bool add_value_to_map(const char *regex_str, std::string errmsg, std::string key);

		bool is_valid_header();
		int content_length_validation();

		int parse_multipart_form();
		std::string get_multipart_form_boundary();

		bool check_multipart_content_type(MultipartFormData &multipart_form_data);
		bool check_multipart_header(MultipartFormData &multipart_form_data);
};

#endif /* REQUEST_PARSER_HPP */