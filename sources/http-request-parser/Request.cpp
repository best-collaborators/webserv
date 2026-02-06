#include "Request.hpp"

Request::Request() : _status_code(0) {}

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
