#ifndef HTTP_REQUEST_HANDLER
#define HTTP_REQUEST_HANDLER

#include "Request.hpp"
#include "ReaderState.hpp"
#include "HeaderState.hpp"
#include "BodyState.hpp"

#include "RequestParser.hpp"
#include "BufferManager.hpp"

class HttpRequestReader
{
private:
	Request		_request;

	ssize_t		_stored_body_bytes;
	ReaderState	_curr_state = ReaderState::AwaitingHeaders;

	HeaderState	_checkHeaderState(std::string &read_buffer) noexcept;
	BodyState	_checkBodyState(std::string &buffer, size_t bytes_read) noexcept;

	void		_handleCompleteBody(std::string &buffer) noexcept;
	BodyState	_handleChunkedBody(std::string &buffer) noexcept;
	
	void		_parseHeaders(std::string &read_buffer) noexcept;
	void		_consumeHeader(std::string &read_buffer) noexcept;

	HeaderState	_handleHeaderMethod(std::string &read_buffer) noexcept;

	bool		_headersComplete(const std::string &read_buffer) const noexcept;
	void		_removeBodyFromBuffer() noexcept;

public:
	HttpRequestReader() = default;
	~HttpRequestReader() = default;

	HttpRequestReader( HttpRequestReader const & ) = delete;
	HttpRequestReader & operator=( HttpRequestReader const & ) = delete;

	HttpRequestReader( HttpRequestReader && ) noexcept = default;
	HttpRequestReader & operator=( HttpRequestReader && ) noexcept = default;

	ReaderState		_processBody(std::string &buffer, size_t bytes_read) noexcept;
	ReaderState		_processHeader(std::string &buffer) noexcept;
	void			reset();

	ReaderState		read(std::string &buffer, size_t bytes_read);
	Request			&request() noexcept;
	ssize_t 		getStoredBodyBytes();
};

#endif /* HTTP_REQUEST_HANDLER */