#include "Request.hpp"

Request::Request() : _status_code(0), _current_chunk_size(0) {}

Request::~Request() { }

uint Request::get_status_code() const {
	return _status_code;
}

void Request::set_status_code(uint status_code) {
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
