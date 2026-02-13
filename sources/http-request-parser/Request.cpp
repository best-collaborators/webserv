#include "Request.hpp"

Request::Request() : _method(HttpMethod::e_code::INVALID), _status_code(HttpStatus::code_from_number(0)), _current_chunk_size(0) {}

HttpStatus::e_code Request::get_status_code() const {
	return _status_code;
}

void Request::set_status_code(uint status_code) {
	_status_code = HttpStatus::code_from_number(status_code);
}

void Request::set_status_code(HttpStatus::e_code status_code) {
	_status_code = status_code;
}

void Request::print_http_request_values() const
{
	for (auto values : _headers) {
		std::cout << "[" << values.first << "] " << "[" << values.second  << "] " << std::endl;
	}
	std::cout << std::endl;
}

std::string Request::get_current_chunk() const
{
	return _current_chunk;
}

size_t Request::get_current_chunk_size() const
{
	return _current_chunk_size;
}

bool Request::is_chunk_received() const
{
	return _chunk_received;
}

void Request::set_is_chunk_received(bool status)
{
	_chunk_received = status;
}

void Request::increase_chunk_size(size_t amount)
{
	_current_chunk_size += amount;
}

void Request::set_chunk_size(size_t amount)
{
	_current_chunk_size = amount;
}

size_t Request::get_current_chunk_size_actual() const
{
	return _current_chunk.size();
}

void Request::set_current_chunk(std::string &&chunk)
{
	_current_chunk = chunk;
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

// bool Request::expectsBody() const
// {
// 	
// }

// bool Request::isChunked() const
// {
// 	return 
// }

// bool Request::isMultipart() const
// {
// 	return ;
// }

bool Request::isStatusCodeBad() const
{
	return HttpStatus::is_bad(_status_code);
}

RequestBodyStatus Request::getBodyStatus() const
{
	if (_method == HttpMethod::e_code::POST)
	{
		return RequestBodyStatus::RAW_BODY;
	}
	else if (get_header_value(http::headers::TRANSFER_ENCODING).find("chunked") != std::string::npos)
	{
		return RequestBodyStatus::CHUNKED;
	}
	else if (getContentType().find("multipart/form-data") != std::string::npos)
	{
		return RequestBodyStatus::MULTIPART;
	}
	return RequestBodyStatus::NO_BODY;
}
