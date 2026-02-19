#include "HttpResponseWriter.hpp"

void HttpResponseWriter::formResponse(Request &_request, CGIExitStatus status, std::string &buffer)
{
	if (status == CGIExitStatus::SUCCESS)
		_response.form_response(_request.get_status_code(), _request.copy_headers(), buffer);
	else
		_response.form_response(_request.get_status_code(), _request.copy_headers());
}

void HttpResponseWriter::formResponse(Request &_request)
{
	_response.form_response(_request.get_status_code(), _request.copy_headers());
}

void	HttpResponseWriter::write()
{
	_response.read_body_partially();
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
