#include "HttpRequestReader.hpp"

bool HttpRequestReader::_headersComplete(const std::string &read_buffer) const noexcept
{
	return read_buffer.find("\r\n\r\n") != std::string::npos;
}

void HttpRequestReader::_parseHeaders(std::string &read_buffer) noexcept
{
	RequestParser header_parser(_request, read_buffer);
	header_parser.parse_headers();

	std::cout << "Header received. Status code -> "
			<< _request.get_status_code() << std::endl;
}

void HttpRequestReader::_consumeHeader(std::string &read_buffer) noexcept
{
	size_t header_end_position = read_buffer.find("\r\n\r\n");
	if (header_end_position != std::string::npos)
		read_buffer.erase(0, header_end_position + 4);
}

HeaderState HttpRequestReader::_handleHeaderMethod(std::string &read_buffer) noexcept
{
	std::string method = _request.get_header_value(http::headers::METHOD);
	if (!HttpMethod::hasBody(_request.get_method())) {

		if (_request.get_content_length() != -1 || read_buffer.size() > 0) {
			std::cout << "[parser] Error (GET/OPTIONS/HEAD requests cannot have body)" << std::endl;
			return HeaderState::Error;
		}
	}
	return HeaderState::Complete;
}

HeaderState HttpRequestReader::_checkHeaderState(std::string &read_buffer) noexcept
{
	if (_curr_state != ReaderState::AwaitingHeaders) return HeaderState::Complete;

	if (!_headersComplete(read_buffer))
		return HeaderState::Incomplete;

	_parseHeaders(read_buffer);
	_consumeHeader(read_buffer);
	if (HttpStatus::is_bad(_request.get_status_code())) {
		return HeaderState::Error;
	}

	_request.print_http_request_values();

	if (_request.get_method() != HttpMethod::e_code::POST && read_buffer.size() != 0) {
		return HeaderState::Error;
	}

	if (_request.getBodyStatus() == RequestType::CGI) {
		return HeaderState::CGI;
	}

	return _handleHeaderMethod(read_buffer);
}

ReaderState HttpRequestReader::_processHeader(std::string &read_buffer) noexcept
{
	switch (_checkHeaderState(read_buffer))
	{
		case HeaderState::Incomplete:
			std::cout << "[request-reader] Request received (partial buffer)." << std::endl;
			return ReaderState::AwaitingHeaders;

		case HeaderState::Complete:
			if (_request.get_method() != HttpMethod::e_code::POST) {
				return ReaderState::Complete;
			}
			return ReaderState::AwaitingBody;

		//! CHECK RETURN STATUS CLOSE
		case HeaderState::Error:
			std::cout << "[request-reader] Request headers invalid." << std::endl;
			return ReaderState::Error;

		case HeaderState::CGI:
			std::cout << "[request-reader] Request is CGI" << std::endl;
			_stored_body_bytes = read_buffer.size();
			return ReaderState::CGI;

		default:
			return ReaderState::Complete;
	}
}

BodyState HttpRequestReader::_checkBodyState(std::string &read_buffer, size_t bytes_read) noexcept
{
	if (bytes_read == 0) {
		return BodyState::Complete;
	}
	_stored_body_bytes = read_buffer.size();
	std::cout << "\n[request-reader] stored_body_bytes: " << _stored_body_bytes << "\n===============\n";

	if (_request.get_header_count(http::headers::TRANSFER_ENCODING)) {
		return BodyState::Chunked;
	}
	if (_stored_body_bytes == _request.get_content_length()) {
		return BodyState::Complete;
	}
	else if ( _request.get_header_count(http::headers::CONTENT_LENGTH) && _stored_body_bytes > _request.get_content_length())
	{
		std::cout << "Read buffer size: " << read_buffer.size() << std::endl;
		_request.set_status_code(HttpStatus::e_code::NOT_FOUND);
		return BodyState::Overflow;
	}
	return BodyState::Incomplete;
}

void HttpRequestReader::_handleCompleteBody(std::string &buffer) noexcept
{
	std::cout << "[request-reader] Request received (complete)." << std::endl;

	RequestParser body_parser(_request, buffer);
	body_parser.parse_body();

	std::cout << "Body received. Status code -> "
			  << _request.get_status_code() << std::endl;
}

BodyState HttpRequestReader::_handleChunkedBody(std::string &buffer) noexcept
{
	std::cout << "[request-reader] Request received (chunked)." << std::endl;

	// std::cout << "_read_buffer is\n"
			// << _read_buffer << std::endl;

	RequestParser body_parser(_request, buffer);
	body_parser.parse_body();

	// std::cout << "Chunk received, chunk size is :"
	// 		<< _request.chunkHandler().getExpectedSize() << std::endl;

	// std::cout << "Body is\n"
	// 		<< _request.get_body() << std::endl;

	// std::cout << "_read_buffer is\n"
	// 		<< _read_buffer << std::endl;

	if (_request.chunkHandler().isReceived()) {

		std::cout << "Body received. Status code -> "
			  << _request.get_status_code() << std::endl;
		return BodyState::Complete;
	}

	return BodyState::Incomplete;
}

ReaderState HttpRequestReader::_processBody(std::string &buffer, size_t bytes_read) noexcept
{
	// std::cout << "body: " << _read_buffer << std::endl;

	switch (_checkBodyState(buffer, bytes_read))
	{
		case BodyState::Incomplete:
			std::cout << "[request-reader] Request received (partial buffer)." << std::endl;
			return ReaderState::AwaitingBody;

		case BodyState::Complete:
			_handleCompleteBody(buffer);
			return ReaderState::Complete;

		case BodyState::Chunked:
		{
			BodyState chunked_body_state = _handleChunkedBody(buffer);
			if (chunked_body_state == BodyState::Complete)
				return ReaderState::Complete;
			return ReaderState::AwaitingBody;
		}

		//! CHECK RETURN STATUS CLOSE
		case BodyState::Overflow:
			std::cout << "[request-reader] Request received. Body too long." << std::endl;
			_request.set_status_code(HttpStatus::e_code::NOT_FOUND);
			return ReaderState::Error;

		case BodyState::Invalid:
			std::cout << "[request-reader] Request received. Request is not suppose to have body." << std::endl;
			_request.set_status_code(HttpStatus::e_code::NOT_FOUND);
			return ReaderState::Error;
	}
	return ReaderState::Complete;
}

void HttpRequestReader::reset()
{
	_curr_state = ReaderState::AwaitingHeaders;
	_stored_body_bytes = 0;
	_request.reset();
}

ReaderState HttpRequestReader::read(std::string &buffer, size_t bytes_read)
{
	if (_curr_state == ReaderState::AwaitingHeaders)
		_curr_state = _processHeader(buffer);

	if (_curr_state == ReaderState::AwaitingBody)
		_curr_state = _processBody(buffer, bytes_read);

	return _curr_state;
}

ssize_t HttpRequestReader::getStoredBodyBytes() const noexcept
{
	return _stored_body_bytes;
}

ssize_t HttpRequestReader::getContentLength() const noexcept
{
	return _request.get_content_length();
}

void HttpRequestReader::setStatusCode(HttpStatus::e_code status)
{
	_request.set_status_code(status);
}

HttpStatus::e_code HttpRequestReader::getStatusCode()
{
	return _request.get_status_code();
}

std::unordered_map<std::string, std::string> HttpRequestReader::moveHeaders()
{
	return _request.copy_headers();
}
