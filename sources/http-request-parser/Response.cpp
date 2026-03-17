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

std::string Response::serve_html_webserv_page(const std::string &msg)
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
		   "    <p>" + (msg.empty() ? HttpStatus::get_message(_status_code) : msg) + "</p>\n"
		   "</body>\n"
		   "</html>\n";
}

Response::e_response_type Response::is_set_default_page(const error_map &error_pages)
{
	auto method = _request->get_method();
	if (error_pages.count(_status_code)) {
		return CUSTOM_ERROR_PAGE;
	}

	if (method == HttpMethod::e_code::OPTIONS || _status_code == HttpStatus::e_code::NO_CONTENT) {
		_status_code = HttpStatus::e_code::NO_CONTENT;
		_body.clear();
	}
	else if (HttpStatus::is_redirect(_status_code)
		  || HttpStatus::is_bad(_status_code)
		  || method == HttpMethod::e_code::POST) {
			return DEFAULT_ERROR_PAGE;
	}
	return FILE;
}

bool Response::is_ifstream_successful(std::ifstream &ifs)
{
	if (ifs.is_open()) return true;

	switch (errno)
	{
		case 2:
			std::cout << "[response] No such file or directory" << std::endl;
			_status_code = HttpStatus::e_code::NOT_FOUND;
			break;
		default:
			std::cout << "[response] File system error" << std::endl;
			_status_code = HttpStatus::e_code::SERVICE_UNAVAILABLE;
			break;
	}
	ifs.close();
	_body = serve_html_webserv_page();
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

void Response::read_body_partially(const std::string &filename)
{
	if (_response_length > 0 &&
		(_body.size() >= _response_length ||
		_body.size() >= _buffer ||
		_bytes_sent > _response_length))
	{
		return;
	}

	std::ifstream ifs (filename, std::ios::binary);
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

std::streampos Response::get_file_size(const std::string &filename)
{
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

std::string Response::form_response( const Request *request, const std::string &body, bool isCGI)
{
	_response_length = 0;
	_content_length = 0;
	_bytes_read = 0;
	_bytes_sent = 0;
	_is_default_page = false;

	_request = request;
	auto file = _request->getFile();
	auto error_pages = _request->getServerBlock()->_error_pages;
	_status_code = _request->get_status_code();

	if (_status_code != HttpStatus::e_code::NO_CONTENT && file.isDir() && file.getAutoindex()) {
		_body = ListingGenerator::getListingPage(file);
		_content_length = _body.size();
	}
	else if (HttpStatus::is_good(_status_code) && isCGI)
	{
		_body = serve_html_webserv_page(body);
		_content_length = _body.size();
	}
	else
	{
		e_response_type response_type = is_set_default_page(error_pages);
		switch (response_type)
		{
			case FILE:
			{
				std::string filename = file.getFullFilename();

				Log::info("File to send back:" + filename, "response");

				get_file_size(filename);
				if (HttpStatus::is_good(_status_code)) {
					set_content_type(filename);
					read_body_partially(filename);
				}
				break;
			}
			case DEFAULT_ERROR_PAGE:
				_body = serve_html_webserv_page();
				_content_length = _body.size();
				break;
			case CUSTOM_ERROR_PAGE:
			{
				std::string filename = error_pages.at(_status_code);
				Log::info("Custom error page: " + filename, "response");
				
				get_file_size(filename);
				if (HttpStatus::is_good(_status_code)) {
					set_content_type(filename);
					read_body_partially(filename);
				}
				break;
			}
			default:
				break;
		}
	}

	std::ostringstream ostringstream;
	std::cout << _status_code << std::endl;

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

	if (HttpStatus::is_redirect(_status_code)) {
		ostringstream << "Location: " << file.getReturnPage().path.string() << "\r\n";
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
