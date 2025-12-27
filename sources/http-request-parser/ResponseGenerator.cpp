#include "ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator(uint status_code, std::string content, std::string_view content_type, bool is_a_file)
	: _status_code(status_code), _content(content), _content_type(content_type), _is_a_file(is_a_file) {}

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

std::string ResponseGenerator::serve_html_error_page(std::string errmsg)
{
	std::string str(HttpStatus::get_status_code_name(static_cast<HttpStatus::e_code>(_status_code)));

	return "<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"	<meta charset=\"UTF-8\">\n"
			"	<title>Error</title>\n"
			"	<style>\n"
			"		body {\n"
			"			font-family: Arial, sans-serif;\n"
			"			background-color: #f8f9fa;\n"
			"			color: #333;\n"
			"			text-align: center;\n"
			"			padding: 50px;\n"
			"		}\n"
			"		h1 {\n"
			"			font-size: 72px;\n"
			"			margin: 0;\n"
			"			color: #e74c3c;\n"
			"		}\n"
			"		h2 {\n"
			"			font-size: 24px;\n"
			"			margin: 10px 0 20px 0;\n"
			"		}\n"
			"		p {\n"
			"			font-size: 16px;\n"
			"			color: #555;\n"
			"		}\n"
			"		a {\n"
			"			color: #3498db;\n"
			"			text-decoration: none;\n"
			"		}\n"
			"		a:hover {\n"
			"			text-decoration: underline;\n"
			"		}\n"
			"		.container {\n"
			"			display: inline-block;\n"
			"			text-align: left;\n"
			"		}\n"
			"	</style>\n"
			"</head>\n"
			"<body>\n"
			"<div class=\"container\">\n"
			"	<h1>" + std::to_string(_status_code) + "</h1>\n"
			"	<h2>" + str + "</h2>\n"
			"	<p>" + errmsg + "</p>\n"
			"	<p><a href=\"/\">Return to Home</a></p>\n"
			"</div>\n"
			"</body>\n"
			"</html>\n";
}

std::string ResponseGenerator::create_body()
{
	if (!_is_a_file) return "";
	if (_status_code > 300) return serve_html_error_page("Sorry.");

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
		std::string temp = serve_html_error_page("Sorry.");
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

	std::string response_file = "./sources/http-parser/test_http_response.txt";
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
