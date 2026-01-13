#include "ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator(uint status_code, std::string content, std::string_view content_type, std::string_view method, bool is_a_file)
	: _status_code(status_code), _content(content), _content_type(content_type), _method(method), _is_a_file(is_a_file) {}

std::string ResponseGenerator::get_file_last_modified_date(const char *filename)
{
	struct stat filestat;
	int code = stat(filename, &filestat);
	if (code < 0) return "";
	return ctime(&filestat.st_mtime);
}

std::_Put_time<char> ResponseGenerator::get_date_GMT()
{
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	return std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT");
}

std::string ResponseGenerator::serve_html_webserv_page(std::string msg)
{
	std::string status_code_message(HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(_status_code)));

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

std::string ResponseGenerator::create_body()
{
	if (!_is_a_file) return "";
	if (_status_code > 300) return serve_html_webserv_page("Error happend.");
	if (_method == "POST") return serve_html_webserv_page("Successfull post.");
	if (_status_code == 304 || _status_code == 204) return serve_html_webserv_page("Other message.");

	std::ifstream ifs (_content);
	char *buffer = nullptr;

	if (!ifs)
	{
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
		std::string temp = serve_html_webserv_page("Sorry.");
		_length = temp.size();
		return temp;
	}

	ifs.seekg(0, ifs.end);
	_length = ifs.tellg();
	ifs.seekg(0, ifs.beg);

	//! CHANGE TO CHUNKS
	if (_length > 0)
	{
		buffer = new char[static_cast<int>(_length) + 1];
		ifs.read(buffer, _length);
		if (!ifs.good()) _status_code = 503;
		buffer[_length] = '\0';
	}
	std::cout << "successful read" << std::endl;

	ifs.close();
	std::string temp(buffer);
	delete [] buffer;
	return temp;
}

void ResponseGenerator::form_reponse()
{
	std::string buffer = create_body().c_str();
	if (!_is_a_file)
	{
		buffer = _content;
		_length = buffer.size();
	}

	std::string response_file = "./tests/test_http_response.txt";
	std::ofstream ofs(response_file);
	if (!ofs)
	{
		std::cerr << std::strerror(errno) << " file: " << response_file;
		return ;
	}

	ofs << "HTTP/1.1"  << " "
		<< _status_code << " "
		<< HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(_status_code)) << std::endl
		<< "Server: webserv/42.0.0" << std::endl
		<< "Date: " << get_date_GMT() << std::endl;

	ofs << "Content-Type: "   << _content_type << std::endl
		<< "Content-Length: " << _length << std::endl;

	//For cache
	if (_is_a_file && _status_code < 300)
		ofs << "Last-Modified: " << get_file_last_modified_date(_content.c_str()) << std::endl;

	ofs << "Connection: close" << std::endl
		<< std::endl << buffer;

	ofs.close();
}
