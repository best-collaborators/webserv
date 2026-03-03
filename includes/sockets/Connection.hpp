#pragma once

#include <chrono>
#include <iostream>
#include <optional>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>

#include "IoResult.hpp"
#include "Socket.hpp"

#include <unordered_map>

#include "Logger.hpp"
#include "Response.hpp"
#include "RequestParser.hpp"
#include "BodyState.hpp"
#include "HeaderState.hpp"
#include "CGIOperation.hpp"
#include "CGIConfig.hpp"
#include "CGIHandler.hpp"
#include "EventAction.hpp"

#include "HttpRequestReader.hpp"
#include "HttpResponseWriter.hpp"
#include "ConnectionState.hpp"
#include "CGIRequestConfig.hpp"

#include "ServerBlock.hpp"

class Connection
{
private:
	using opt_cgi = std::optional<CGIHandler>;

	std::chrono::time_point<std::chrono::steady_clock>	_last_activity;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>>	_cgi_start_time;

	int			_fd;
	
	Socket		_socket;
	opt_cgi		_cgi_handler;
	ServerBlock const * _server_block;

	BufferManager _buffer_manager;
	HttpRequestReader _request_reader;
	HttpResponseWriter _response_writer;

	bool _response_formed = false;
	bool _headers_sent_to_client = false;

	ssize_t		_sent_bytes;
	ssize_t		_read_bytes;

	IoEvent		_getSocketState() const noexcept;
	IoEvent		_tryInitCGI() noexcept;

	HttpStatus::e_code	_validateCGIOutput( std::string & cgi_buffer ) noexcept;

	IoEvent		_receiveData() noexcept;
	IoEvent		_handleReceiveState( ssize_t read_bytes ) noexcept;

	IoEvent		_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;
	void		_formResponse();
	void		_formCGIResponse();
	IoEvent		_sendData() noexcept;

public:
	Connection() = default;
	Connection( ServerBlock const * server_block, Socket && socket );

	Connection( Connection const & ) = delete;
	Connection & operator=( Connection const & ) = delete;

	Connection( Connection && ) noexcept = default;
	Connection & operator=( Connection && ) noexcept = default;

	~Connection() = default;

	int			getFD() const noexcept;
	void		abortCGI() noexcept;
	void		abortCGIWithError() noexcept;
	bool		headersSentToClient() const noexcept;

	IoResult	processConnectionEvents( uint32_t const events );
	IoResult	processCGIEvents( uint32_t const events );

	int			getCGIPID() const noexcept;
	int			getCGIPipe( CGIOperation op );
	void		closeCGIPipe( CGIOperation op );
	void		resetLastActivity() noexcept;

	EventAction	onCGIOutputReady();
	EventAction	onChildProcessExited();
	std::chrono::time_point<std::chrono::steady_clock>	getLastActivity() const noexcept;
	std::optional<std::chrono::time_point<std::chrono::steady_clock>>	getCGIStartTime() const noexcept;
};