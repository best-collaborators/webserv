#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <iostream>
#include <unordered_map>
#include <string>
#include "HttpMessage.hpp"

class Request : public HttpMessage
{
private:
	uint _status_code;

public:
	Request();
	Request(const Request &other) = default;
	Request(Request &&other) = default;
	Request & operator=( Request && ) noexcept = default;
	~Request();

	uint get_status_code() const;
	void set_status_code(uint status_code);
	void print_http_request_values() const;
};

#endif /* HTTP_REQUEST */