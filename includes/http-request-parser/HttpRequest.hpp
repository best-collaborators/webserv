#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <iostream>
#include <unordered_map>
#include <string>
#include "HttpMessage.hpp"

class HttpRequest : public HttpMessage
{
private:
	uint _status_code;

public:
	HttpRequest(uint _status_code, std::unordered_map<std::string, std::string> &&_http_request_values);
	HttpRequest() = default;
	~HttpRequest();

	uint get_status_code() const;
	void set_status_code(uint status_code);
	void print_http_request_values() const;
};

#endif /* HTTP_REQUEST */