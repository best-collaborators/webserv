#pragma once

#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <chrono>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "Socket.hpp"
#include "Logger.hpp"
#include "IoResult.hpp"
#include "Response.hpp"
#include "RequestParser.hpp"
#include "BodyState.hpp"
#include "HeaderState.hpp"
#include "CGIOperation.hpp"
#include "CGIConfig.hpp"
#include "CGIHandler.hpp"
#include "CGIValidator.hpp"
#include "EventAction.hpp"
#include "ServerBlock.hpp"
#include "ConnectionState.hpp"
#include "CGIRequestConfig.hpp"
#include "HttpRequestReader.hpp"
#include "HttpResponseWriter.hpp"

class Connection
{
public:
	using time_point = std::chrono::time_point<std::chrono::steady_clock>;
	using opt_time = std::optional<time_point>;

	opt_time	_cgi_start_time;

	Connection() = delete;
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
	time_point	getLastActivity() const noexcept;
	opt_time	getCGIStartTime() const noexcept;

private:
	using opt_cgi = std::optional<CGIHandler>;

	int					_fd;
	
	Socket				_socket;
	opt_cgi				_cgi_handler;

	BufferManager		_buffer_manager;
	HttpRequestReader	_request_reader;
	HttpResponseWriter	_response_writer;

	time_point			_last_activity;

	bool				_response_formed = false;
	bool				_headers_sent_to_client = false;

	ssize_t				_sent_bytes = 0;
	ssize_t				_read_bytes = 0;

	IoEvent				_getSocketState() const noexcept;

	IoEvent				_tryInitCGI() noexcept;

	IoEvent				_receiveData() noexcept;
	IoEvent				_handleReceiveState( ssize_t read_bytes ) noexcept;

	IoEvent				_sendData() noexcept;
	IoEvent				_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;

	void				_formResponse();
	void				_formCGIResponse();
};