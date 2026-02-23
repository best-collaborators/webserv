#include "Request.hpp"

Request::Request() : _method(HttpMethod::e_code::INVALID), _status_code(HttpStatus::code_from_number(0)) {}

HttpStatus::e_code Request::get_status_code() const
{
	return _status_code;
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


bool Request::isStatusCodeBad() const
{
	return HttpStatus::is_bad(_status_code);
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
	_version.clear();
	_uri.clear();
	_chunk_handler.reset();
	_headers.clear();
	_body.clear();
}