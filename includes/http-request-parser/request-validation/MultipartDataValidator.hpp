#ifndef MULTIPART_DATA_VALIDATOR
#define MULTIPART_DATA_VALIDATOR

#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <regex>

#include "RequestStringUtils.hpp"
#include "MultipartFormData.hpp"
#include "HttpRegexPatterns.hpp"
#include "Trimmer.hpp"

#include "HttpStatus.hpp"

class MultipartDataValidator
{
private:
	std::vector <MultipartFormData>		_multipartFormDatas;
	std::string							_content_type;
	std::string							_request;

public:
	MultipartDataValidator(std::vector <MultipartFormData> &_multipartFormDatas,
		std::string &content_type,
		std::string &_request
	);
	~MultipartDataValidator();

	MultipartDataValidator() = delete;
	MultipartDataValidator(const MultipartDataValidator &other) = delete;
	MultipartDataValidator(MultipartDataValidator &&other) = delete;
	MultipartDataValidator & operator=( MultipartDataValidator && ) noexcept = delete;

	std::string			get_multipart_form_boundary();

	HttpStatus::e_code	parse_multipart_data_form();
	HttpStatus::e_code	create_multipart_data_form_files();
	HttpStatus::e_code	parse_multipart_form_data(std::string &buffer, MultipartFormData &multipart_form_data);

	HttpStatus::e_code	truncate_boundary(
		std::string &buffer,
		bool &is_end,
		std::string &boundary_marker,
		std::string &closing_boundary_marker,
		MultipartFormData &multipart_form_data
	);

	bool				check_multipart_content_type(const std::string &buffer, MultipartFormData &multipart_form_data);
	bool				check_multipart_header(const std::string &buffer, MultipartFormData &multipart_form_data);
};

#endif /* MULTIPART_DATA_VALIDATOR */