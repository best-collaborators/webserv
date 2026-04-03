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
	_content_type = HttpContentType::e_code::TEXT_HTML;

	if (should_generate_autoindex()) {

		auto file = _request->getFile();
		std::string listening_page = ListingGenerator::getListingPage(file);
		if (!listening_page.empty()) {
			_content_length = _body.size();
			_status_code = HttpStatus::e_code::OK;
			return listening_page;
		}
		_status_code = HttpStatus::e_code::NOT_FOUND;
	}

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

bool Response::is_ifstream_successful(std::ifstream &ifs, std::filesystem::path path)
{
	if (ifs.is_open()) return true;

	switch (errno)
	{
		case 2:
			Log::error("No such file or directory "
				+ std::filesystem::weakly_canonical(path).string(), "response");
			_status_code = HttpStatus::e_code::NOT_FOUND;
			break;
		default:
			Log::error("File system error"
				+ std::filesystem::weakly_canonical(path).string(), "response");
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

	if (std::filesystem::is_directory(filename)) {
		_status_code = HttpStatus::e_code::NOT_FOUND;
		return;
	}

	std::ifstream ifs (filename, std::ios::binary);
	if (!is_ifstream_successful(ifs, filename)) return;

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
	_content_type = HttpContentType::to_code(filename);
}

std::streampos Response::get_file_size(const std::string &filename)
{
	std::filesystem::path normalized_path = std::filesystem::weakly_canonical(filename);
	if (std::filesystem::is_directory(normalized_path)) {
		_status_code = HttpStatus::e_code::NOT_FOUND;
		_body = serve_html_webserv_page("Is a directory.");
		_content_length = _body.size();
		return 0;
	}

	std::ifstream ifs(filename, std::ios::binary);
	if (!is_ifstream_successful(ifs, filename)) {
		Log::warning("Impossible to retrieve size of " + normalized_path.string());
		return 0;
	}

	std::streampos fbegin = ifs.tellg();
	ifs.seekg(0, ifs.end);
	_content_length = ifs.tellg() - fbegin;
	ifs.close();
	return _content_length;
}

std::string Response::form_response(const Request *request, const std::string &body, bool isCGI)
{
	init_response(request);

	if (is_success() && isCGI)
		handle_cgi(body);
	else
		handle_regular_response();

	build_headers();
	build_full_response();

	return _body;
}

void Response::init_response(const Request* request)
{
	_response_length = 0;
	_content_length = 0;
	_bytes_read = 0;
	_bytes_sent = 0;
	_is_default_page = false;

	_request = request;
	_status_code = _request->get_status_code();
}

bool Response::should_generate_autoindex()
{
	auto file = _request->getFile();
	return (_status_code != HttpStatus::e_code::METHOD_NOT_ALLOWED
			&& _status_code != HttpStatus::e_code::NO_CONTENT
			&& !HttpStatus::is_redirect(_status_code)
			&& file.isDir() && file.getAutoindex());
}

bool Response::is_success()
{
	return HttpStatus::is_good(_status_code);
}

void Response::handle_cgi(const std::string& body)
{
	_body = serve_html_webserv_page(body);
	_content_length = _body.size();
}

void Response::handle_regular_response()
{
	auto file = _request->getFile();
	error_map error_pages = _request->getServerBlock()->_error_pages;

	if (HttpStatus::is_redirect(_status_code)) {
		return ;
	}

	if (_request->get_method() == HttpMethod::e_code::OPTIONS)
		_status_code = HttpStatus::e_code::NO_CONTENT;

	if (_status_code == HttpStatus::e_code::NO_CONTENT
		|| _status_code == HttpStatus::e_code::CREATED) {

		_body.clear();
		_content_length = 0;
		return;
	}

	if (is_success())
		serve_file(file.getFullFilename());
	else
		serve_error_page(error_pages);
}

void Response::serve_file(const std::string& filename)
{
	Log::debug("File to send back: " + filename, "response");

	set_content_type(filename);
	get_file_size(filename);
	read_body_partially(filename);
}

void Response::serve_error_page(const error_map& error_pages)
{
	std::string filename;

	if (error_pages.count(_status_code))
		filename = error_pages.at(_status_code);

	if (!filename.empty() && is_valid_file(filename))
	{
		set_content_type(filename);
		get_file_size(filename);
		read_body_partially(filename);
	}
	else
	{
		_body = serve_html_webserv_page();
		_content_length = _body.size();
		_content_type = HttpContentType::e_code::TEXT_HTML;
	}
}

bool Response::is_valid_file(const std::string& filename)
{
	std::filesystem::path path = std::filesystem::weakly_canonical(filename);
	std::ifstream ifs(path, std::ios::binary);

	return ifs.good() && !std::filesystem::is_directory(path);
}

void Response::build_headers()
{
	std::ostringstream oss;

	oss << "HTTP/1.1 " << _status_code << "\r\n"
		<< "Server: webserv/42.0.0\r\n"
		<< "Access-Control-Allow-Origin: *\r\n"
		<< "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
		<< "Access-Control-Allow-Headers: Content-Type, X-Filename\r\n"
		<< "Date: " << get_date_GMT() << "\r\n";

	if (_status_code != HttpStatus::e_code::NO_CONTENT &&
		_status_code != HttpStatus::e_code::CREATED)
	{
		oss << "Content-Type: "
			<< HttpContentType::to_string(_content_type) << "\r\n";
		}
	oss << "Content-Length: " << _content_length << "\r\n";

	if (HttpStatus::is_redirect(_status_code))
		oss << "Location: "
			<< _request->getFile().getReturnPage().path.string() << "\r\n";

	oss << "Connection: "
		<< (HttpStatus::is_good(_status_code) ? "Keep-Alive" : "Close")
		<< "\r\n\r\n";

	_header_str = oss.str();
}

void Response::build_full_response()
{
	std::ostringstream oss;

	oss << _header_str;

	if (!_body.empty())
	{
		oss << _body;
	}

	_body = oss.str();
	_response_length = _header_str.size() + _content_length;
	Log::debug("RESPONSE: " + _body, "http-parser");
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
