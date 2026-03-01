#pragma once

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
#include "CGIExitStatus.hpp"
#include "EventAction.hpp"

#include "HttpRequestReader.hpp"
#include "HttpResponseWriter.hpp"
#include "ConnectionState.hpp"
#include "CGIRequestConfig.hpp"

class Connection
{
private:
	using opt_cgi = std::optional<CGIHandler>;

	int			_fd;
	
	Socket		_socket;
	opt_cgi		_cgi_handler;

	BufferManager _buffer_manager;
	HttpRequestReader _request_reader;
	HttpResponseWriter _response_writer;

	bool _response_formed = false;

	ssize_t		_sent_bytes;
	ssize_t		_read_bytes;

	IoEvent		_getSocketState() const noexcept;
	IoEvent		_tryInitCGI() noexcept;

	IoEvent		_receiveData() noexcept;
	IoEvent		_handleReceiveState( ssize_t read_bytes ) noexcept;

	IoEvent		_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;
	void		_formResponse();
	IoEvent		_sendData() noexcept;

public:
	Connection() = default;
	Connection( Socket && socket );

	Connection( Connection const & ) = delete;
	Connection & operator=( Connection const & ) = delete;

	Connection( Connection && ) noexcept = default;
	Connection & operator=( Connection && ) noexcept = default;

	~Connection() = default;

	int			getFD() const noexcept;

	IoResult	processConnectionEvents( uint32_t const events );
	IoResult	processCGIEvents( uint32_t const events );

	int			getCGIPID() const noexcept;
	int			getCGIPipe( CGIOperation op );
	void		closeCGIPipe( CGIOperation op );

	EventAction	onCGIOutputReady();
	EventAction	onChildProcessExited( ChildExitInfo const & info );
	
};