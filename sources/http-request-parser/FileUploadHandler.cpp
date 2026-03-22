#include "FileUploadHandler.hpp"

size_t FileUploadHandler::_uploaded_files_count = 0;
bool FileUploadHandler::_initialized = false;

FileUploadHandler::FileUploadHandler( fs::path upload_dir, Request &request, std::string filename ) : 
_upload_dir(upload_dir),  _filename(filename), _request(request) {

	if (!_initialized) initialize_count(upload_dir);
}

void FileUploadHandler::_getFileName()
{
	if (!_filename.empty()) {
		_filename = _upload_dir / _filename;
		return ;
	}

	const File &file = _request.getFile();
	if (!file.isDir()) {
		_filename = file.getFullFilename();
		return;
	}

	if (_request.get_header_count("x-filename")) {
		_filename = _request.get_header_value("x-filename");
	}
	else {
		_filename = std::to_string(_uploaded_files_count % http::limits::upload_file_modulo) + "-upload";

		const std::string content_type = _request.getContentType();

		std::string extension;
		if (content_type.empty())
			extension = ".bin";
		else 
			extension = HttpContentType::get_extension_by_content_type(content_type);
		_filename.replace_extension(extension);
	}
	_filename = _upload_dir / _filename;
}

void FileUploadHandler::write_into_file( const std::string &_body )
{
	if (HttpStatus::is_bad(_request.get_status_code()))
		return ;

	_getFileName();

	std::cerr << "[http] Writing into " << _filename << std::endl;
	std::fstream fout(_filename, std::ios::binary | std::ios::out);
	if (!fout) {
		std::cerr << "[http] Error happened while writing into " << _filename << std::endl;
		_request.set_status_code(HttpStatus::e_code::INTERNAL_SERVER_ERROR);
		return ;
	}

	fout.write(_body.c_str(), _body.size());

	_uploaded_files_count++;
	_request.set_status_code(HttpStatus::e_code::CREATED);
	fout.close();
}

void FileUploadHandler::initialize_count(const std::string& dir)
{
	try
	{
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
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
	_initialized = true;
}
