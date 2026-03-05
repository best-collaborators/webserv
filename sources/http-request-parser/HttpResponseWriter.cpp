#include "HttpResponseWriter.hpp"

void HttpResponseWriter::formResponse(
	HttpStatus::e_code status_code,
	std::unordered_map<std::string, std::string> &&_headers,
	const std::optional<std::string> & buffer)
{
	if (buffer.has_value())
		_response.form_response(status_code, std::move(_headers), buffer.value());
	else
		_response.form_response(status_code, std::move(_headers), "");
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
