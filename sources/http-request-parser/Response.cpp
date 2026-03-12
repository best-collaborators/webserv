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
    set_header_value(http::headers::CONTENT_TYPE, "text/html");

    return "<!DOCTYPE html>\n"
           "<html lang=\"en\">\n"
           "<head>\n"
           "    <meta charset=\"UTF-8\">\n"
           "    <title>" +
           std::to_string(HttpStatus::number_from_code(_status_code)) + " " +
           HttpStatus::get_status_code_name(_status_code) +
           "</title>\n"
           "</head>\n"
           "<body>\n"
           "    <center>\n"
           "        <h1>" +
           std::to_string(HttpStatus::number_from_code(_status_code)) + " " +
           HttpStatus::get_status_code_name(_status_code) +
           "</h1>\n"
           "    </center>\n"
           "    <hr><center>webserv/42.0.0</center>\n"
           "    <p>" + msg + "</p>\n"
           "</body>\n"
           "</html>\n";
}

void Response::is_set_default_page()
{
	std::string method = get_header_value(http::headers::METHOD);
	if (method == "OPTIONS" || _status_code == HttpStatus::e_code::NO_CONTENT) {
		_status_code = HttpStatus::e_code::NO_CONTENT;
		_body = "";
	}
	else if (HttpStatus::is_redirect(_status_code)) {
		_body = serve_html_webserv_page("You've been redirected.");
	}
	else if (HttpStatus::is_bad(_status_code)) {
		_body = serve_html_webserv_page("Error happened.");
	}
	else if (method == "POST") {
		_body = serve_html_webserv_page("Successful post.");
	}
	else {
		_is_default_page = false;
		return ;
	}
	_content_length = _body.size();
	_is_default_page = true;
}

bool Response::is_ifstream_successful(std::ifstream &ifs)
{
	if (ifs.is_open()) return true;

	switch (errno)
	{
		case 2:
			//No such file or directory
			std::cout << "[response] No such file or directory" << std::endl;
			_status_code = HttpStatus::e_code::NOT_FOUND;
			break;
		case 13:
			//Permission denied
			std::cout << "[response] File system error" << std::endl;
			_status_code = HttpStatus::e_code::SERVICE_UNAVAILABLE;
			break;
		default:
			std::cout << "[response] File system error" << std::endl;
			_status_code = HttpStatus::e_code::SERVICE_UNAVAILABLE;
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
	if (_response_length > 0 &&
		(_body.size() >= _response_length ||
		_body.size() >= _buffer ||
		_bytes_sent > _response_length))
	{
		return;
	}

	std::ifstream ifs (_file.getFullFilename(), std::ios::binary);
	if (!is_ifstream_successful(ifs)) return;

	const std::size_t current_size = _body.size();
	const std::size_t size_to_read = _buffer - current_size;

	std::vector<char> buffer(size_to_read);

	ifs.seekg(get_file_read_position());
	ifs.read(buffer.data(), size_to_read);
	const std::streamsize curr_bytes_read = ifs.gcount();

	if (curr_bytes_read > 0)
	{
		_body.append(buffer.data(), curr_bytes_read);
		_bytes_read += curr_bytes_read;
	}

	if (!ifs && _bytes_sent < _response_length) {
		_status_code = HttpStatus::e_code::SERVICE_UNAVAILABLE;
		return;
	}
	ifs.close();
}

void Response::set_content_type(std::string filename)
{
	std::filesystem::path path = filename;
	auto extension = path.extension();
	set_header_value(http::headers::CONTENT_TYPE, HttpContentType::get_content_type_by_extension(extension.string()));
}

std::streampos Response::get_file_size()
{
	std::string filename = _file.getFullFilename();
	std::ifstream ifs(filename, std::ios::binary);
	if (!is_ifstream_successful(ifs)) {
		std::cerr << "[response] Impossible to retrieve size of " << filename << std::endl;
		return 0;
	}
	std::streampos fbegin = ifs.tellg();
	ifs.seekg(0, ifs.end);
	_content_length = ifs.tellg() - fbegin;
	ifs.close();
	return _content_length;
}

std::string Response::form_response( const HttpStatus::e_code &status_code, const HttpMethod::e_code &method, const File &file, const std::string &body)
{
	_response_length = 0;
	_content_length = 0;
	_bytes_read = 0;
	_bytes_sent = 0;

	_status_code = status_code;
	_method = method;
	_file = file;

	std::cout << "filename: " << file.getFullFilename() << std::endl;
	if (status_code != HttpStatus::e_code::NO_CONTENT && file.getAutoindex()) {
		_body = serve_html_webserv_page("IT'S DIRECTORY LISTENING");
		_content_length = _body.size();
	}
	else if (HttpStatus::is_good(status_code) && !body.empty())
	{
		_body = serve_html_webserv_page(body);
		_content_length = _body.size();
	}
	else
	{
		is_set_default_page();

		if (!_is_default_page && (_file.getFullFilename()).size() < 2) {
			_status_code = HttpStatus::e_code::OK;
			_body = serve_html_webserv_page("Root not configured");
			_is_default_page = true;
			_content_length = _body.size();
		}

		if (!_is_default_page)
		{
			std::cout << "[response] Not a default page" << std::endl;
			std::cout << "[response] file to send back: " << _file.getFullFilename() << std::endl;
			get_file_size();
			if (HttpStatus::is_good(_status_code)) {
				set_content_type(_file.getFullFilename());
				read_body_partially();
			}
		}
	}

	//* TODO: AFTER CONFIGURATION FILE IS CREATED ADJUST THIS TO WORK WITH STRING NOT ONLY FILE
	// if (!_is_a_file)
	// {
	// 	buffer = _parse_result.get_content();
	// 	_response_length = buffer.size();
	// }

	std::ostringstream ostringstream;
	std::cout << status_code << std::endl;

	ostringstream << "HTTP/1.1"  << " "
		<< _status_code << "\r\n"
		<< "Server: webserv/42.0.0\r\n"
		<< "Access-Control-Allow-Origin: *\r\n"
		<< "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
  		<< "Access-Control-Allow-Headers: Content-Type, X-Filename\r\n"
		<< "Date: " << get_date_GMT() << "\r\n";

	if (!_body.empty()) {
		ostringstream  << "Content-Type: " << get_header_value(http::headers::CONTENT_TYPE) << "\r\n";
		ostringstream << "Content-Length: " << _content_length << "\r\n";
	}

	if (HttpStatus::is_redirect(status_code)) {
		ostringstream << "Location: " << _file.getReturnPage().path.string() << "\r\n";
	}

	//For cache
	// if (get_header_value(http::headers::METHOD) != "POST" && _status_code != HttpStatus::e_code::CREATED &&/*  _status_code < 300 */)
	// 	ostringstream << "Last-Modified: " << get_file_last_modified_date(get_header_value(http::headers::REQUEST_TARGET)) << "\r\n";

	if (HttpStatus::is_bad(_status_code))
		ostringstream << "Connection: close" << "\r\n";

	ostringstream << "\r\n";
	_header_str = ostringstream.str();

	if (!_body.empty())
		ostringstream << _body;

	_body = ostringstream.str();

	_response_length = _header_str.size() + _content_length;
	// std::cout << "content length" << _content_length << std::endl;
	std::cout << "RESPONSE:\n" << std::quoted(_body) << std::endl;
	// std::cout << "header size:                  ==> \n" << _header_str.size() << std::endl;
	// std::cout << "size:                  ==> " << _body.size() << std::endl;

	return _body;
}

HttpStatus::e_code Response::status_code() const noexcept 
{
	return _status_code;
}

size_t Response::get_total_response_length() const noexcept
{
	return _response_length;
}

size_t Response::get_current_length() const noexcept 
{
	return _body.size();
}

const char *Response::getResponseData() const noexcept
{
	return _body.c_str();
}

void Response::consume_body(size_t consume_length)
{
	if (consume_length > _body.size())
		consume_length = _body.size();
	_body.erase(0, consume_length);
	_bytes_sent += consume_length;
}
