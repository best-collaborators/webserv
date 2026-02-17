#pragma once

#include <iostream>
#include <optional>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>

#include "IoState.hpp"
#include "Socket.hpp"

#include <unordered_map>

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

class Connection
{
private:
	using opt_cgi = std::optional<CGIHandler>;

	int			_fd;
	
	Socket		_socket;
	opt_cgi		_cgi_handler;

	ConnectionState _state;
	BufferManager _buffer_manager;
	HttpRequestReader _request_reader;
	HttpResponseWriter _response_writer;

	ssize_t		_sent_bytes;
	size_t		_read_bytes;

	IoState		_receiveData() noexcept;
	IoState		_sendData() noexcept;

	IoState		_handleReceiveState( ssize_t read_bytes ) noexcept;
	IoState		_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;

	IoState		_getSocketState() const noexcept;

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