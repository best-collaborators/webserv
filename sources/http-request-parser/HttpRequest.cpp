#include "HttpRequest.hpp"

HttpRequest::HttpRequest(uint status_code, std::unordered_map<std::string, std::string> &&http_request_values)
: HttpMessage(http_request_values),
_status_code(status_code) {}

HttpRequest::~HttpRequest() { }

uint HttpRequest::get_status_code() const {
	return _status_code;
}

void HttpRequest::set_status_code(uint status_code) {
	_status_code = status_code;
}

void HttpRequest::print_http_request_values() const
{
	for (auto values : _headers) {
		std::cout << "[" << values.first << "] " << "[" << values.second  << "] " << std::endl;
	}
}