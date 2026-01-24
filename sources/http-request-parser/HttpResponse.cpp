#include "HttpResponse.hpp"

HttpResponse::HttpResponse(uint status_code, std::unordered_map<std::string, std::string> &&http_request_values)
: HttpMessage(http_request_values),
_status_code(status_code) {}

HttpResponse::~HttpResponse() { }

uint HttpResponse::get_status_code() const {
	return _status_code;
}

void HttpResponse::set_status_code(uint status_code) {
	_status_code = status_code;
}