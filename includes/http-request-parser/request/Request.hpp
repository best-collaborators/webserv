#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <iostream>
#include <unordered_map>
#include <string>
#include "HttpMessage.hpp"

class Request : public HttpMessage
{
private:
	uint		_status_code;
	std::string	_current_chunk;
	size_t		_current_chunk_size;
	bool		_chunk_received;

public:
	Request();
	Request(const Request &other) = default;
	Request(Request &&other) = default;
	Request & operator=( Request && ) noexcept = default;
	~Request();

	uint get_status_code() const;
	void set_status_code(uint status_code);
	void print_http_request_values() const;

	std::string get_current_chunk() const;
	size_t get_current_chunk_size() const;
	bool	is_chunk_received() const;
	void	set_is_chunk_received(bool status);
};

#endif /* HTTP_REQUEST */