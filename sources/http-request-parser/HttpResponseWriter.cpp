#include "HttpResponseWriter.hpp"

void HttpResponseWriter::formResponse(
	const Request *request,
	const std::string & buffer,
	bool isCGI)
{
	_response.form_response(request, buffer, isCGI);
}

void	HttpResponseWriter::write( const Request *request )
{
	_response.read_body_partially(request->getFile().getFullFilename());
}

size_t HttpResponseWriter::totalLength() const noexcept
{
	return _response.get_total_response_length();
}

size_t HttpResponseWriter::currResponseLength() const noexcept
{
	return _response.get_current_length();
}

const char *HttpResponseWriter::getResponseData() const noexcept
{
	return _response.getResponseData();
}

void HttpResponseWriter::consume(size_t bytes) noexcept
{
	return _response.consume_body(bytes);
}
