#ifndef HTTP_RESPONSE_WRITER_HPP
#define HTTP_RESPONSE_WRITER_HPP

#include "Response.hpp"

class HttpResponseWriter
{
private:
	bool		_response_formed = false;
	Response	_response;

public:
	HttpResponseWriter(/* args */);
	~HttpResponseWriter();

	void formResponse();
};

#endif /* HTTP_RESPONSE_WRITER_HPP */
