#include "Response.hpp"

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

void Response::is_set_default_page()
{
	std::string method = get_header_value("method");
	if (method == "OPTIONS" || _status_code == 204) {
		_status_code = 204;
		_body = "";
	}
	else if (method == "POST") {
		_body = serve_html_webserv_page("Successfull post.");
	}
	else if (_status_code > 300) {
		_body = serve_html_webserv_page("Error happend.");
	}
	else if (_status_code == 304) {
		_body = serve_html_webserv_page("Other message.");
	}
	else {
		_is_default_page = false;
		return ;
	}
	_content_length = _body.size();
	_is_default_page = true;
}

bool Response::is_fstream_successful(std::fstream &ifs)
{
	if (ifs.is_open()) return true;

	switch (errno)
	{
		case 2:
			//No such file or directory
			std::cout << "[response] No such file or directory" << std::endl;
			_status_code = 404;
			break;
		case 13:
			//Permission denied
			std::cout << "[response] Permission denied" << std::endl;
			_status_code = 503;
			break;
		default:
			std::cout << "[response] Permission denied" << std::endl;
			_status_code = 503;
			break;
	}
	ifs.close();
	_body = serve_html_webserv_page("Sorry.");
	_content_length = _body.size();
	return false;
}

std::streampos Response::get_file_read_position()
{
	if (_bytes_read > _content_length)
		return std::streampos(_content_length - (_bytes_read - _content_length));
	else
		return std::streampos(_bytes_read);
}

void Response::read_body_partially()
{
	if (_response_length > 0 && (_body.size() == _response_length || _body.size() >= _buffer || _bytes_sent > _response_length)) return ;

	std::string filename = _root + get_header_value("request-target");
	std::fstream ifs (filename, std::ios::binary | std::ios::in);
	if (!is_fstream_successful(ifs)) return;

	ssize_t size_to_add = _buffer - _body.size();
	char buffer[size_to_add];

	std::streampos file_pos = get_file_read_position();
	ifs.seekg(file_pos);
	ifs.read(buffer, size_to_add);

	std::streamsize gcount = ifs.gcount();
	buffer[gcount] = '\0';
	_body.append(buffer, gcount);

	_bytes_read += gcount;

	if (!ifs && _bytes_sent < _response_length) {
		_status_code = 503;
		return;
	}
	ifs.close();
}

void Response::set_content_type(std::string filename)
{
	std::filesystem::path path = filename;
	auto extension = path.extension();
	set_header_value("content-type", HttpContentType::get_content_type_by_extension(extension.string()));
}

std::streampos Response::get_file_size()
{
	std::string filename = _root + get_header_value("request-target");
	std::fstream ifs(filename, std::ios::in | std::ios::binary);
	if (!is_fstream_successful(ifs)) {
		std::cerr << "[response] Impossible to retrieve size of " << filename << std::endl;
		_status_code = 503;
		return 0;
	}
	std::streampos fbegin = ifs.tellg();
	ifs.seekg(0, ifs.end);
	_content_length = ifs.tellg() - fbegin;
	ifs.close();
	return _content_length;
}

std::string Response::form_response(uint status_code, std::unordered_map<std::string, std::string> &&http_request_values, std::string body)
{
	_status_code = status_code;
	_response_length = 0;
	_content_length = 0;
	_bytes_read = 0;
	_bytes_sent = 0;

	set_header_value("request-target", http_request_values["request-target"]);
	set_header_value("method", http_request_values["method"]);

	is_set_default_page();

	if (!_is_default_page && (get_header_value("request-target")).size() < 2) {
		_status_code = 200;
		_body = serve_html_webserv_page("Root not configured");
		_is_default_page = true;
		_content_length = _body.size();
	}

	if (!_is_default_page)
	{
		std::cout << "[response] Not a default page" << std::endl;
		std::cout << "[response] file to send back: " << _root + get_header_value("request-target") << std::endl;
		get_file_size();
		if (_status_code < 300) {
			set_content_type(get_header_value("request-target"));
			if (body.empty())
				read_body_partially();
			else
				_body = serve_html_webserv_page(body);
		}
	}

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
  		<< "Access-Control-Allow-Headers: Content-Type, X-Filename\r\n"
		<< "Date: " << get_date_GMT() << "\r\n";

	if (!_body.empty()) {
		ostringstream  << "Content-Type: " << get_header_value("content-type") << "\r\n";
		ostringstream << "Content-Length: " << _content_length << "\r\n";
	}

	//For cache
	// if (get_header_value("method") != "POST" && _status_code != 201 && _status_code < 300)
	// 	ostringstream << "Last-Modified: " << get_file_last_modified_date(get_header_value("request-target")) << "\r\n";

	if (_status_code > 400)
		ostringstream << "Connection: close" << "\r\n";

	ostringstream << "\r\n";
	_header_str = ostringstream.str();

	if (!_body.empty())
		ostringstream << _body;

	_body = ostringstream.str();

	_response_length = _header_str.size() + _content_length;
	// std::cout << "content length" << _content_length << std::endl;
	// std::cout << "body:                  ==> \n" << _body << std::endl;
	// std::cout << "header size:                  ==> \n" << _header_str.size() << std::endl;
	// std::cout << "size:                  ==> " << _body.size() << std::endl;

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

void Response::consume_body(size_t consume_length)
{
	if (consume_length > _body.size())
		consume_length = _body.size();
	_body.erase(0, consume_length);
	_bytes_sent += consume_length;
}

std::string &Response::get_body()
{
	return _body;
}
