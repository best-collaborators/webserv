#ifndef MULTIPART_DATA_VALIDATOR
#define MULTIPART_DATA_VALIDATOR

#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <regex>

#include "FileUploadHandler.hpp"

#include "RequestStringUtils.hpp"
#include "MultipartFormData.hpp"
#include "HttpRegexPatterns.hpp"
#include "Trimmer.hpp"

#include "HttpStatus.hpp"
#include "IParser.hpp"
#include "ParseContext.hpp"

class MultipartDataParser : public IParser
{
private:
	struct BoundaryContext
	{
		std::string boundary;
		std::string boundary_marker;
		std::string closing_boundary_marker;
		bool is_end;
	};

	std::vector <MultipartFormData>		_multipartFormDatas;
	ParseContext						&_parse_context;
	BoundaryContext						_boundary_context;

	bool _isBoundaryEmpty();
	bool _isValidMultipartForm();

	std::string			_getMultipartFormBoundary();
	void				_createMultipartDataFormFiles();
	HttpStatus::e_code	_parseMultipartFormData(std::string &buffer, MultipartFormData &multipart_form_data);
	bool				_checkMultipartContentType(const std::string &buffer, MultipartFormData &multipart_form_data);
	bool				_checkMultipartHeader(const std::string &buffer, MultipartFormData &multipart_form_data);

	HttpStatus::e_code	truncate_boundary(
								std::string &buffer,
								BoundaryContext &boundary_context,
								MultipartFormData &multipart_form_data
							);

	MultipartDataParser() = delete;
	MultipartDataParser(const MultipartDataParser &other) = delete;
	MultipartDataParser(MultipartDataParser &&other) = delete;
	MultipartDataParser & operator=( MultipartDataParser && ) noexcept = delete;

public:
	MultipartDataParser(std::vector <MultipartFormData> &_multipartFormDatas,
		ParseContext &parse_context
	);
	~MultipartDataParser() = default;
	void				parse();

};

#endif /* MULTIPART_DATA_VALIDATOR */