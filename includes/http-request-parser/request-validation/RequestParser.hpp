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
#include "Request.hpp"
#include "RequestGenerator.hpp"

class RequestParser
{
private:
	std::string										_buffer;
	std::vector<MultipartFormData>					_multipartFormDatas;
	Request											&_request;
	std::string										_raw_bits;

	//! Move to a different class
	uint											_uploaded_files_count;

	const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
	const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
	const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

	const char *REGEX_HTTP_METHOD = "(^[A-Z]{1,32}[ ]+)";
	const char *REGEX_HTTP_REQUEST_TARGET = "(^/\\S*[\\s]+)";
	const char *REGEX_HTTP_VESRION = "(HTTP\\/(\\d)+.(\\d)+\\s*$)";
	const char *REGEX_HTTP_HEADER = "(^\\S{1,256}:[ ]+)";

	std::string		get_regex_value(std::string &line, std::regex regex_method);

	bool			is_valid_request_line();
	uint			validate_request_line();

	bool			add_value_to_map(const char *regex_str, std::string errmsg, std::string key);

	bool			is_valid_header();
	int				content_length_validation();

	uint			validate_request_headers();
	uint			validate_request_body();

public:
	RequestParser(Request &request, std::string &raw_bits);
	RequestParser(const RequestParser &other) = default;
	RequestParser(RequestParser &&other) = default;
	RequestParser & operator=( RequestParser && ) noexcept = default;
	~RequestParser() = default;

	void			parse_body();
	void			parse_headers();
	uint			get_status_code();
};

#endif /* REQUEST_PARSER_HPP */