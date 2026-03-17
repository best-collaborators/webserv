#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <iostream>
#include <unordered_map>
#include <string>
#include "HttpMessage.hpp"
#include "HttpStatus.hpp"

#include "HttpMethod.hpp"
#include "HttpHeaders.hpp"

#include "RequestBodyStatus.hpp"
#include "ChunkHandler.hpp"
#include "ServerBlock.hpp"
#include "File.hpp"

class Request : public HttpMessage
{
private:
	HttpMethod::e_code	_method;
	std::string			_version;
	std::string			_uri;

	HttpStatus::e_code	_status_code;
	ChunkHandler		_chunk_handler;
	bool				_is_cgi;

	ServerBlock const * _server_block;
	File				_file;

public:
	Request(ServerBlock const * server_block);
	Request() = default;
	Request(const Request &other) = default;
	Request(Request &&other) = default;
	Request & operator=( Request && ) noexcept = default;
	~Request() = default;

	HttpStatus::e_code	 get_status_code() const;
	void				 set_status_code(HttpStatus::e_code status_code);

	HttpMethod::e_code	 get_method() const;
	void				 set_method(std::string method);

	const File			&getFile() const;
	void				setFile(const File &file);

	std::string			 getContentType() const;

	void				 print_http_request_values() const;

	bool				 has_body_required_headers() const;

	ChunkHandler&		 chunkHandler();
	void				 reset();

	bool				isGoodStatusCode() const;

	bool				isCGI();
	void				setIsCGI(bool is_cgi);

	void				adjustHeaderForCGI();

	ServerBlock const *	getServerBlock() const;
	RequestType 		getBodyStatus() const;
};

#endif /* HTTP_REQUEST */