#ifndef MULTIPART_DATA_VALIDATOR
#define MULTIPART_DATA_VALIDATOR

#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <regex>

#include "ValidatorHelpers.hpp"
#include "MultipartFormData.hpp"
#include "HttpRegexPatterns.hpp"
#include "Trimmer.hpp"

class MultipartDataValidator
{
private:
	std::vector <MultipartFormData>		_multipartFormDatas;
	std::string							_content_type;
	std::string							_buffer;
	std::string							_request;

public:
	MultipartDataValidator(std::vector <MultipartFormData> &_multipartFormDatas,
		std::string &content_type,
		std::string &_buffer,
		std::string &_request
	);
	~MultipartDataValidator();

	MultipartDataValidator() = delete;
	MultipartDataValidator(const MultipartDataValidator &other) = delete;
	MultipartDataValidator(MultipartDataValidator &&other) = delete;
	MultipartDataValidator & operator=( MultipartDataValidator && ) noexcept = delete;

	std::string			get_multipart_form_boundary();

	uint				parse_multipart_data_form();
	uint				create_multipart_data_form_files();
	uint				parse_multipart_form_data(MultipartFormData &multipart_form_data);

	uint				truncate_boundary(bool &is_end,
		std::string &boundary_marker,
		std::string &closing_boundary_marker,
		MultipartFormData &multipart_form_data
	);

	bool				check_multipart_content_type(MultipartFormData &multipart_form_data);
	bool				check_multipart_header(MultipartFormData &multipart_form_data);
};

#endif /* MULTIPART_DATA_VALIDATOR */