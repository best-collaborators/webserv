#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

#include <iostream>
#include <unordered_map>
#include <string>
#include "HttpMessage.hpp"

class HttpResponse : public HttpMessage
{
private:
	uint _status_code;

public:
	HttpResponse(uint _status_code, std::unordered_map<std::string, std::string> &&_http_request_values);
	HttpResponse() = default;

	HttpResponse(HttpResponse &other) = default;
	HttpResponse(HttpResponse &&other) = default;
	HttpResponse & operator=( HttpResponse && ) noexcept = default;
	~HttpResponse();

	uint get_status_code() const;
	void set_status_code(uint status_code);
};

#endif /* HTTP_RESPONSE */