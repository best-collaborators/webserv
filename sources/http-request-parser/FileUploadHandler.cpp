#include "FileUploadHandler.hpp"

size_t FileUploadHandler::_uploaded_files_count = 0;

FileUploadHandler::FileUploadHandler( std::string upload_dir, Request &request ) : 
_upload_dir(upload_dir), _filename(""), _request(request) {

	if (!_initialized) initialize_count(upload_dir);
}

void FileUploadHandler::write_into_file( std::string &_body )
{
	if (HttpStatus::is_bad(_request.get_status_code()))
		return ;

	if (_request.get_header_count("x-filename")) {
		_filename = _request.get_header_value(_request.get_header_value("x-filename"));
	}
	else {
		std::string content_type = _request.getContentType();

		if (content_type.empty())
			_filename = std::to_string(_uploaded_files_count % http::limits::upload_file_modulo) + "-upload..bin";
		else 
			_filename = std::to_string(_uploaded_files_count % http::limits::upload_file_modulo) + "-upload" + HttpContentType::get_extension_by_content_type(content_type);
	}

	std::string full_filename = std::string(_upload_dir) + std::string(_filename);

	std::fstream fout(full_filename, std::ios::binary | std::ios::out);
	if (!fout) {
		std::cerr << "[http] Error happened while writing into " << _filename << std::endl;
		_request.set_status_code(500);
		return ;
	}

	fout.write(_body.c_str(), _body.size());

	_uploaded_files_count++;
	_request.set_status_code(201);
	fout.close();
}

void FileUploadHandler::initialize_count(const std::string& dir)
{
	try
	{
		for (const auto& entry : std::filesystem::directory_iterator("data")) {
			if (std::filesystem::is_regular_file(entry.status())) {
				++_uploaded_files_count;
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "[data] Cannot retrieve amount of uploaded files" << std::endl;
		// _request.set_status_code(500);
	}
}
