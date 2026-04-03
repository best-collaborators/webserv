#ifndef HTTP_RESPONSE_WRITER_HPP
#define HTTP_RESPONSE_WRITER_HPP

#include <optional>

#include "Response.hpp"
#include "Request.hpp"

class HttpResponseWriter
{
private:
	Response		_response;

public:
	HttpResponseWriter() = default;
	~HttpResponseWriter() = default;

	HttpResponseWriter( HttpResponseWriter const & ) = delete;
	HttpResponseWriter & operator=( HttpResponseWriter const & ) = delete;

	HttpResponseWriter( HttpResponseWriter && ) noexcept = default;
	HttpResponseWriter & operator=( HttpResponseWriter && ) noexcept = default;
	
	void formResponse( const Request *request, const std::string & buffer = "", bool isCGI = false);

	void		write( const Request *request );
	size_t		totalLength() const noexcept;
	size_t		currResponseLength() const noexcept;
	const char	*getResponseData() const noexcept;
	void		consume(size_t bytes) noexcept;
};

#endif /* HTTP_RESPONSE_WRITER_HPP */
