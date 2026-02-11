#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <iostream>
#include <unordered_map>
#include <string>
#include "HttpMessage.hpp"
#include "HttpStatus.hpp"

class Request : public HttpMessage
{
private:
	HttpStatus::e_code	_status_code;
	std::string			_current_chunk;
	size_t				_current_chunk_size;
	bool				_chunk_received;

public:
	Request();
	Request(const Request &other) = default;
	Request(Request &&other) = default;
	Request & operator=( Request && ) noexcept = default;
	~Request();

	HttpStatus::e_code	 get_status_code() const;
	void				 set_status_code(uint status_code);
	void				 set_status_code(HttpStatus::e_code status_code);
	void				 print_http_request_values() const;

	std::string 		 get_current_chunk() const;
	size_t				 get_current_chunk_size_actual() const;

	size_t get_current_chunk_size() const;
	bool	is_chunk_received() const;
	
	
	void	set_chunk_size(size_t amount);
	void	set_current_chunk(std::string &&chunk);
	void	set_is_chunk_received(bool status);
	void	increase_chunk_size(size_t amount);
};

#endif /* HTTP_REQUEST */