#include "HttpRequestReader.hpp"

HttpRequestReader::HttpRequestReader(ServerBlock const * server_block) : _request(server_block) {}

bool HttpRequestReader::_headersComplete(const std::string &read_buffer) const noexcept
{
	return read_buffer.find("\r\n\r\n") != std::string::npos;
}

void HttpRequestReader::_parseHeaders(std::string &read_buffer) noexcept
{
	RequestParser header_parser(_request, read_buffer);
	header_parser.parse_headers();

	#ifdef DDEBUG_FLAG
		std::cout << "Header received. Status code -> "
				<< _request.get_status_code() << std::endl;
	#endif
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

		if (_request.get_content_length() != 0 || read_buffer.size() > 0) {
			Log::error("Error (GET/OPTIONS/HEAD requests cannot have body):\n" + read_buffer, "parser");
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
	if (HttpStatus::is_bad(_request.get_status_code())) {
		_consumeHeader(read_buffer);
		return HeaderState::Error;
	}

	_request.print_http_request_values();

	if (HttpStatus::is_redirect(_request.get_status_code())) {
		_consumeHeader(read_buffer);
		return HeaderState::Redirect;
	}

	Log::debug("_request.isCGI() " + std::to_string(_request.isCGI()), "parser");
	if (_request.isCGI() && _request.get_method() != HttpMethod::e_code::POST) return HeaderState::CGI;
	return _handleHeaderMethod(read_buffer);
}

ReaderState HttpRequestReader::_processHeader(std::string &read_buffer) noexcept
{
	switch (_checkHeaderState(read_buffer))
	{
		case HeaderState::Incomplete:
			Log::debug("Request received (partial buffer)." + std::to_string(_request.isCGI()), "request-reader");
			return ReaderState::AwaitingHeaders;

		case HeaderState::Complete:
			if (_request.get_method() != HttpMethod::e_code::POST) {
				return ReaderState::Complete;
			}
			return ReaderState::AwaitingBody;

		case HeaderState::Redirect:
			return ReaderState::Complete;

		case HeaderState::Error:
			Log::error("Request headers invalid." + std::to_string(_request.isCGI()), "request-reader");
			return ReaderState::Error;

		case HeaderState::CGI:
			Log::debug("Request is CGI." + std::to_string(_request.isCGI()), "request-reader");
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

	if (_request.get_header_count(http::headers::CONTENT_LENGTH)) {
		if (_stored_body_bytes >= _request.get_content_length()) {
			return BodyState::Complete;
		} else {
			return BodyState::Incomplete;
		}
	}
	if (_request.get_header_count(http::headers::TRANSFER_ENCODING)) {
		return BodyState::Chunked;
	}
	if (_stored_body_bytes == _request.get_content_length()) {
		return BodyState::Complete;
	}
	else if (_request.get_header_count(http::headers::CONTENT_LENGTH) && _stored_body_bytes > _request.get_content_length())
	{
		Log::debug("Read buffer size: " + std::to_string(read_buffer.size()), "request-reader");
		_request.set_status_code(HttpStatus::e_code::PAYLOAD_TOO_LARGE);
		return BodyState::Overflow;
	}
	else if (_stored_body_bytes > _request.getFile().getMaxBodySize())
	{
		Log::debug("Read buffer size: " + std::to_string(read_buffer.size()), "request-reader");
		_request.set_status_code(HttpStatus::e_code::PAYLOAD_TOO_LARGE);
		return BodyState::Overflow;
	}
	return BodyState::Incomplete;
}

void HttpRequestReader::_handleCompleteBody(std::string &buffer) noexcept
{
	Log::debug("Request received (complete).", "request-reader");

	RequestParser body_parser(_request, buffer);
	body_parser.parse_body();

	#ifdef DDEBUG_FLAG
		std::cout << "Body received. Status code -> "
				<< _request.get_status_code() << std::endl;
	#endif
}

BodyState HttpRequestReader::_handleChunkedBody(std::string &buffer) noexcept
{
	Log::debug("Request received (chunked).", "request-reader");

	RequestParser body_parser(_request, buffer);
	body_parser.parse_body();

	if (_request.chunkHandler().isReceived()) {
		#ifdef DDEBUG_FLAG
			std::cout << "Body received. Status code -> "
					<< _request.get_status_code() << std::endl;
		#endif
		if (HttpStatus::is_good(_request.get_status_code()) && _request.isCGI()) {
			return BodyState::CGI;
		}
		return BodyState::Complete;
	}
	return BodyState::Incomplete;
}

ReaderState HttpRequestReader::_processBody(std::string &buffer, size_t bytes_read) noexcept
{
	switch (_checkBodyState(buffer, bytes_read))
	{
		case BodyState::Incomplete:
			Log::debug("Request received (partial buffer)", "request-reader");
			return ReaderState::AwaitingBody;

		case BodyState::Complete:
			if (_request.isCGI())
			{
				Log::debug("Request received (CGI)", "request-reader");
				return ReaderState::CGI;
			}
			_handleCompleteBody(buffer);
			Log::debug("Request received (complete buffer)", "request-reader");
			return ReaderState::Complete;

		case BodyState::Chunked:
		{
			BodyState chunked_body_state = _handleChunkedBody(buffer);
			Log::debug("Request received (chunk buffer)", "request-reader");
			if (chunked_body_state == BodyState::CGI)
			{
				Log::debug("Request received (CGI)", "request-reader");
				_request.adjustHeaderForCGI();
				buffer = _request.get_body();
				return ReaderState::CGI;
			}
			if (chunked_body_state == BodyState::Complete) {
				Log::debug("Request received (completed chunked)", "request-reader");
				return ReaderState::Complete;
			}
			return ReaderState::AwaitingBody;
		}

		//! CHECK RETURN STATUS CLOSE
		case BodyState::Overflow:
			Log::debug("Request received. Body too long.", "request-reader");
			_request.set_status_code(HttpStatus::e_code::PAYLOAD_TOO_LARGE);
			return ReaderState::Error;

		case BodyState::Invalid:
			Log::debug("Request received. Request is not suppose to have body.", "request-reader");
			_request.set_status_code(HttpStatus::e_code::NOT_FOUND);
			return ReaderState::Error;
		
		default: 
			return ReaderState::Complete;
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

size_t HttpRequestReader::getStoredBodyBytes() const noexcept
{
	return _stored_body_bytes;
}

const Request *HttpRequestReader::request()
{
	return &_request;
}

size_t HttpRequestReader::getContentLength() const noexcept
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

std::unordered_map<std::string, std::string> HttpRequestReader::getHeaders()
{
	return _request.get_headers();
}

void HttpRequestReader::printHeaders()
{
	_request.print_http_request_values();
}

const File& HttpRequestReader::getFile() const
{
	return _request.getFile();
}

HttpMethod::e_code HttpRequestReader::getMethod() const
{
	return _request.get_method();
}