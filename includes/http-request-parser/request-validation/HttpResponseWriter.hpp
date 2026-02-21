#ifndef HTTP_RESPONSE_WRITER_HPP
#define HTTP_RESPONSE_WRITER_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "CGIExitStatus.hpp"

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
	
	void formResponse(HttpStatus::e_code status_code, std::unordered_map<std::string, std::string> &&_headers, CGIExitStatus status, std::string &buffer);
	void formResponse(HttpStatus::e_code status_code, std::unordered_map<std::string, std::string> &&_headers);

	void		write();
	size_t		totalLength() const noexcept;
	size_t		currResponseLength() const noexcept;
	const char	*getResponseData() const noexcept;
	void		consume(size_t bytes) noexcept;
};

#endif /* HTTP_RESPONSE_WRITER_HPP */
