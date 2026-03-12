#include "Request.hpp"

Request::Request(ServerBlock const * server_block) : _method(HttpMethod::e_code::INVALID), _status_code(HttpStatus::code_from_number(0)), _server_block(server_block) {}

HttpStatus::e_code Request::get_status_code() const
{
	return _status_code;
}

ServerBlock const * Request::getServerBlock()
{
	return _server_block;
}

void Request::set_status_code(HttpStatus::e_code status_code)
{
	_status_code = status_code;
}

void Request::print_http_request_values() const
{
	for (auto values : _headers) {
		std::cout << "[" << values.first << "] " << "[" << values.second  << "] " << std::endl;
	}
	std::cout << std::endl;
}


HttpMethod::e_code Request::get_method() const
{
	return _method;
}

void	Request::set_method(std::string method)
{
	_method = HttpMethod::fromString(method);
}

bool Request::has_body_required_headers() const
{
	return get_header_count(http::headers::CONTENT_LENGTH)
		|| get_header_count(http::headers::TRANSFER_ENCODING);
}

std::string Request::getContentType() const
{
	return get_header_value(http::headers::CONTENT_TYPE);
}

bool Request::isGoodStatusCode() const
{
	return HttpStatus::is_good(_status_code);
}

bool Request::isCGI()
{
	return _is_cgi;
}

void Request::setIsCGI(bool is_cgi)
{
	_is_cgi = is_cgi;
}

void Request::adjustHeaderForCGI()
{
	std::cout << "ADJUST HEADERS" << std::endl;
	set_header_value(http::headers::CONTENT_LENGTH, std::to_string(_body.size()));
	remove_header_key(http::headers::TRANSFER_ENCODING);
}

RequestType Request::getBodyStatus() const
{
	if (_method == HttpMethod::e_code::POST && get_header_value(http::headers::TRANSFER_ENCODING).find("chunked") != std::string::npos)
	{
		return RequestType::CHUNKED;
	}
	else if (_method == HttpMethod::e_code::POST && getContentType().find("multipart/form-data") != std::string::npos)
	{
		return RequestType::MULTIPART;
	}
	else if (_method == HttpMethod::e_code::POST)
	{
		return RequestType::RAW_BODY;
	}
	return RequestType::NO_BODY;
}

ChunkHandler& Request::chunkHandler() 
{
	return _chunk_handler;
}

void Request::reset()
{
	_status_code = HttpStatus::e_code::UNKNOWN;
	_method = HttpMethod::e_code::INVALID;
	_is_cgi = false;
	_version.clear();
	_uri.clear();
	_chunk_handler.reset();
	_headers.clear();
	_body.clear();
	_file = File();
}

const File& Request::getFile() const
{
	return _file;
}

void Request::setFile(const File& file)
{
	_file = file;
}
