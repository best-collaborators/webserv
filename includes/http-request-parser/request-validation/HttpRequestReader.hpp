#ifndef HTTP_REQUEST_HANDLER
#define HTTP_REQUEST_HANDLER

#include "Request.hpp"
#include "ReaderState.hpp"
#include "HeaderState.hpp"
#include "BodyState.hpp"

#include "RequestParser.hpp"
#include "BufferManager.hpp"
#include "ServerBlock.hpp"

class HttpRequestReader
{
private:
	Request		_request;

	ssize_t		_stored_body_bytes = 0;
	ReaderState	_curr_state = ReaderState::AwaitingHeaders;

	HeaderState	_checkHeaderState(std::string &read_buffer) noexcept;
	BodyState	_checkBodyState(std::string &buffer, size_t bytes_read) noexcept;

	void		_handleCompleteBody(std::string &buffer) noexcept;
	BodyState	_handleChunkedBody(std::string &buffer) noexcept;
	
	void		_parseHeaders(std::string &read_buffer) noexcept;
	void		_consumeHeader(std::string &read_buffer) noexcept;

	HeaderState	_handleHeaderMethod(std::string &read_buffer) noexcept;

	bool		_headersComplete(const std::string &read_buffer) const noexcept;

public:
	HttpRequestReader() = delete;
	~HttpRequestReader() = default;

	HttpRequestReader( HttpRequestReader const & ) = delete;
	HttpRequestReader & operator=( HttpRequestReader const & ) = delete;

	HttpRequestReader( HttpRequestReader && ) noexcept = default;
	HttpRequestReader & operator=( HttpRequestReader && ) noexcept = default;

	HttpRequestReader(ServerBlock const * _server_block);

	ReaderState			_processBody(std::string &buffer, size_t bytes_read) noexcept;
	ReaderState			_processHeader(std::string &buffer) noexcept;
	void				reset();

	ReaderState			read(std::string &buffer, size_t bytes_read);
	ssize_t 			getStoredBodyBytes() const noexcept;
	ssize_t				getContentLength() const noexcept;

	void				setStatusCode(HttpStatus::e_code status);
	HttpStatus::e_code	getStatusCode();

	std::unordered_map<std::string, std::string> getHeaders();
	std::unordered_map<std::string, std::string> moveHeaders();

	void printHeaders();
};

#endif /* HTTP_REQUEST_HANDLER */