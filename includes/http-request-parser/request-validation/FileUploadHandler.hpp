#ifndef FILE_UPLOAD_HANDLER
#define FILE_UPLOAD_HANDLER

#include "Request.hpp"
#include "HttpLimits.hpp"
#include <iostream>
#include <fstream>

class FileUploadHandler
{
private:
	static size_t		_uploaded_files_count;
	std::string_view	_upload_dir;
	std::string_view	_filename;
	Request 			&_request;

	FileUploadHandler(const FileUploadHandler && other) = delete;
	FileUploadHandler(const FileUploadHandler & other) = delete;

public:
	FileUploadHandler( std::string upload_dir, Request &_request );
	~FileUploadHandler() = default;

	void write_into_file( std::string &_body );
};

#endif /* FILE_UPLOAD_HANDLER */
