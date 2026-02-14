#ifndef FILE_UPLOAD_HANDLER
#define FILE_UPLOAD_HANDLER

#include "Request.hpp"
#include "HttpLimits.hpp"
#include <iostream>
#include <fstream>

class FileUploadHandler
{
private:
	const std::string	_upload_dir;
	std::string			_filename;
	Request 			&_request;

	static size_t		_uploaded_files_count;
	static bool			_initialized;

	FileUploadHandler(const FileUploadHandler && other) = delete;
	FileUploadHandler(const FileUploadHandler & other) = delete;

	void _getFileName();
public:
	FileUploadHandler( std::string upload_dir, Request &request, std::string filename = "" );
	~FileUploadHandler() = default;

	void write_into_file( const std::string &_body );
	static void initialize_count(const std::string& dir);
};

#endif /* FILE_UPLOAD_HANDLER */
