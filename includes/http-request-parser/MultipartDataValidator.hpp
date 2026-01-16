#ifndef MULTIPART_DATA_VALIDATOR
#define MULTIPART_DATA_VALIDATOR

#include <iostream>
#include <regex>
#include "MultipartFormData.hpp"
#include "Trimmer.hpp"
#include <fstream>
#include <vector>

#include "RequestParser.hpp"
#include "ValidatorHelpers.hpp"

class MultipartDataValidator
{
private:
	const char *REGEX_BOUNDARY = "^multipart/form-data;\\s*boundary=([^;\\s]+$)";
	const char *REGEX_CONTENT_TYPE = "^[C,c]ontent-[T,t]ype:\\s*(\\S{1,256}\\/\\S{1,256})\\s*";
	const char *REGEX_CONTENT_DISPOSITION = "^[C,c]ontent-[D,d]isposition: form-data;\\s*name=\"(\\S{1,256})\";?\\s*(filename=\"(\\S{1,256})\")?";

	std::vector <MultipartFormData> _multipartFormDatas;
	std::unordered_map<std::string, std::string> &_http_request_values;
	std::string _buffer;
	std::string _request;

public:
	MultipartDataValidator(std::vector <MultipartFormData> &_multipartFormDatas,
		std::unordered_map<std::string, std::string> &_http_request_values,
		std::string &_buffer,
		std::string &_request
	);
	MultipartDataValidator() = delete;
	~MultipartDataValidator();

	std::string get_multipart_form_boundary();

	uint parse_multipart_data_form();
	uint create_multipart_data_form_files();
	uint parse_multipart_form_data(MultipartFormData &multipart_form_data);

	uint truncate_boundary(bool &is_end, std::string &boundary_marker, std::string &closing_boundary_marker, MultipartFormData &multipart_form_data);

	bool check_multipart_content_type(MultipartFormData &multipart_form_data);
	bool check_multipart_header(MultipartFormData &multipart_form_data);
};

#endif /* MULTIPART_DATA_VALIDATOR */