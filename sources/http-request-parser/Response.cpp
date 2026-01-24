#include "Response.hpp"

Response::Response(uint status_code, std::unordered_map<std::string, std::string> http_request_values)
: _reponse(status_code, std::move(http_request_values)) { }
Response::~Response() { }

std::string Response::get_file_last_modified_date(const std::string *filename)
{
	const char * filename_c = filename->c_str();
	struct stat filestat;
	int code = stat(filename_c, &filestat);
	if (code < 0) return "";
	return ctime(&filestat.st_mtime);
}

std::_Put_time<char> Response::get_date_GMT()
{
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	return std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT");
}

std::string Response::serve_html_webserv_page(std::string msg)
{
	std::string status_code_message(HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(_reponse.get_status_code())));

	_reponse.set_header_value("content-type", "text/html");
	return "<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"	<meta charset=\"UTF-8\">\n"
			"	<title>" + std::to_string(_reponse.get_status_code()) + " " + status_code_message + "</title>\n"
			"</head>\n"
			"<body>\n"
			"	<center><h1>" + std::to_string(_reponse.get_status_code()) + " " + status_code_message + "</h1></center>\n"
			"	<hr><center>webserv/42.0.0</center>"
			"	<p>" + msg + "</p>\n"
			"</body>\n"
			"</html>\n";
}

bool Response::is_set_default_page()
{
	if (*_reponse.get_header_value("method") == "OPTIONS") {
		_body_content = serve_html_webserv_page("Method options.");
	}
	else if (_reponse.get_status_code() > 300) {
		_body_content = serve_html_webserv_page("Error happend.");
	}
	else if (*_reponse.get_header_value("method") == "POST") {
		_body_content = serve_html_webserv_page("Successfull post.");
	}
	else if (_reponse.get_status_code() == 304 || _reponse.get_status_code() == 204) {
		_body_content = serve_html_webserv_page("Other message.");
	}
	else
		return false;
	return true;
}

bool Response::is_fstream_successful(std::fstream &ifs)
{
	std::cout << _root + *_reponse.get_header_value("request-target") << std::endl;
	if (ifs.is_open()) return true;

	switch (errno)
	{
		case 2:
			//No such file or directory
			_reponse.set_status_code(404);
			break;
		case 13:
			//Permission denied
			_reponse.set_status_code(503);
			break;
		default:
			_reponse.set_status_code(503);
			break;
	}
	ifs.close();
	_body_content = serve_html_webserv_page("Sorry.");
	return false;
}

void Response::create_body()
{
	if (is_set_default_page()) return;

	//? TEMP FIX FOR ROOT PATH
	std::cout << "METHOD: " << _reponse.get_status_code() << std::endl;
	std::cout << "CONTENT: " << _reponse.get_header_value("request-target") << std::endl;
	if ((*_reponse.get_header_value("request-target")).size() < 2) {
		_reponse.set_status_code(503);
		_body_content = serve_html_webserv_page("Root not configured"); return ;
	}

	std::string filename = _root + *_reponse.get_header_value("request-target");
	std::fstream ifs (filename, std::ios::binary | std::ios::in);
	if (!is_fstream_successful(ifs)) return;

	while (ifs)
	{
		char buffer[36500];
		ifs.read(buffer, 36500);

		std::streamsize gcount = ifs.gcount();
		_body_content.append(buffer, gcount);

		if (ifs.eof() || !ifs.good()) break;
	}

	ifs.close();
}

std::string Response::form_reponse()
{
	_response_length = 0;
	create_body();
	//* TODO: AFTER CONFIGURATION FILE IS CREATED ADJUST THIS TO WORK WITH STRING NOT ONLY FILE
	// if (!_is_a_file)
	// {
	// 	buffer = _parse_result.get_content();
	// 	_response_length = buffer.size();
	// }

	std::ostringstream ostringstream;
	uint status_code =  _reponse.get_status_code();

	ostringstream << "HTTP/1.1"  << " "
		<< status_code << " "
		<< HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(status_code)) << "\r\n"
		<< "Server: webserv/42.0.0\r\n"
		<< "Access-Control-Allow-Origin: *\r\n"
		<< "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
  		<< "Access-Control-Allow-Headers: Content-Type\r\n"
		<< "Date: " << get_date_GMT() << "\r\n";

	ostringstream  << "Content-Type: " << _reponse.get_header_value("content-type") << "\r\n" 
		<< "Content-Length: " << _body_content.size() << "\r\n";

	//For cache
	if (*_reponse.get_header_value("method") != "POST" && status_code != 201 && status_code < 300)
		ostringstream << "Last-Modified: " << get_file_last_modified_date(_reponse.get_header_value("request-target")) << "\r\n";

	ostringstream << "Connection: close" << "\r\n\r\n" << _body_content;

	return ostringstream.str();
}

uint Response::status_code()
{
	return _reponse.get_status_code();
}