#include "Response.hpp"

// Response::Response(uint status_code, std::unordered_map<std::string, std::string> http_request_values)
// : HttpMessage(std::move(http_request_values)), _status_code(status_code) { }

std::string Response::get_file_last_modified_date(const std::string &filename)
{
	const char * filename_c = filename.c_str();
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
	std::string status_code_message(HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(_status_code)));

	set_header_value("content-type", "text/html");
	return "<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"	<meta charset=\"UTF-8\">\n"
			"	<title>" + std::to_string(_status_code) + " " + status_code_message + "</title>\n"
			"</head>\n"
			"<body>\n"
			"	<center><h1>" + std::to_string(_status_code) + " " + status_code_message + "</h1></center>\n"
			"	<hr><center>webserv/42.0.0</center>"
			"	<p>" + msg + "</p>\n"
			"</body>\n"
			"</html>\n";
}

bool Response::is_set_default_page()
{
	std::string method = get_header_value("method");
	if (method == "OPTIONS") {
		_body = serve_html_webserv_page("Method options.");
	}
	else if (method == "POST") {
		_body = serve_html_webserv_page("Successfull post.");
	}
	else if (_status_code > 300) {
		_body = serve_html_webserv_page("Error happend.");
	}
	else if (_status_code == 304 || _status_code == 204) {
		_body = serve_html_webserv_page("Other message.");
	}
	else
		return false;
	return true;
}

bool Response::is_fstream_successful(std::fstream &ifs)
{
	std::cout << _root + get_header_value("request-target") << std::endl;
	if (ifs.is_open()) return true;

	switch (errno)
	{
		case 2:
			//No such file or directory
			_status_code = 404;
			break;
		case 13:
			//Permission denied
			_status_code = 503;
			break;
		default:
			_status_code = 503;
			break;
	}
	ifs.close();
	_body = serve_html_webserv_page("Sorry.");
	return false;
}

void Response::create_body()
{
	if (is_set_default_page()) return;

	//? TEMP FIX FOR ROOT PATH
	if ((get_header_value("request-target")).size() < 2) {
		_status_code = 503;
		_body = serve_html_webserv_page("Root not configured"); return ;
	}

	std::string filename = _root + get_header_value("request-target");
	std::fstream ifs (filename, std::ios::binary | std::ios::in);
	if (!is_fstream_successful(ifs)) return;

	while (ifs)
	{
		char buffer[36500];
		ifs.read(buffer, 36500);

		std::streamsize gcount = ifs.gcount();
		_body.append(buffer, gcount);

		if (ifs.eof()) break;

		if (!ifs) {
			_status_code = 503;
			_body = serve_html_webserv_page("Sorry.");
			return;
		}
	}
	std::filesystem::path path = filename;
	auto extension = path.extension();
	set_header_value("content-type", HttpContentType::get_content_type_by_extension(extension.string()));

	ifs.close();
}

std::string Response::form_response(uint status_code, std::unordered_map<std::string, std::string> &&http_request_values)
{
	_response_length = 0;
	create_body();
	_status_code = status_code;
	set_headers(std::move(http_request_values));
	
	//* TODO: AFTER CONFIGURATION FILE IS CREATED ADJUST THIS TO WORK WITH STRING NOT ONLY FILE
	// if (!_is_a_file)
	// {
	// 	buffer = _parse_result.get_content();
	// 	_response_length = buffer.size();
	// }

	std::ostringstream ostringstream;

	ostringstream << "HTTP/1.1"  << " "
		<< _status_code << " "
		<< HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(_status_code)) << "\r\n"
		<< "Server: webserv/42.0.0\r\n"
		<< "Access-Control-Allow-Origin: *\r\n"
		<< "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
  		<< "Access-Control-Allow-Headers: Content-Type\r\n"
		<< "Date: " << get_date_GMT() << "\r\n";

	ostringstream  << "Content-Type: " << get_header_value("content-type") << "\r\n"
		<< "Content-Length: " << _body.size() << "\r\n";

	//For cache
	if (get_header_value("method") != "POST" && _status_code != 201 && _status_code < 300)
		ostringstream << "Last-Modified: " << get_file_last_modified_date(get_header_value("request-target")) << "\r\n";

	ostringstream << "Connection: close" << "\r\n\r\n" << _body;

	_body = ostringstream.str();
	_response_length = _body.size();
	return _body;
}

uint Response::status_code()
{
	return _status_code;
}

size_t Response::get_total_response_length()
{
	return _response_length;
}

size_t Response::get_current_length()
{
	return _body.size();
}

// void Response::set_response_length(size_t response_length)
// {
// 	_response_length = response_length;
// }

void Response::consume_body(size_t consume_length)
{
	if (consume_length > _body.size())
		consume_length = _body.size();
	_body.erase(consume_length);
}

std::string &Response::get_body()
{
	return _body;
}