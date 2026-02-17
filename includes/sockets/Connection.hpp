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

class Connection
{
private:
	static constexpr int	READ_BUFFER_SIZE = 32768;

	int			_fd;
	bool		is_header_received = false;
	ssize_t		_stored_body_bytes;
	Request		_request;
	Response	_response;

	Socket		_socket;
	std::optional<CGIHandler> _cgi_handler;

	bool		_response_formed = false;

	char		_recv_buffer[READ_BUFFER_SIZE];
	ssize_t		_read_bytes;
	ssize_t		_sent_bytes;
	std::string	_read_buffer;

	BodyState	_checkBodyState() noexcept;
	void		_handleCompleteBody() noexcept;
	IoState		_processBody() noexcept;

	IoState		_processHeader() noexcept;
	bool		_headersComplete() const noexcept;
	void		_parseHeaders() noexcept;
	void		_consumeHeader() noexcept;
	HeaderState	_handleHeaderMethod() noexcept;
	HeaderState	_checkHeaderState() noexcept;
	void		_removeBodyFromBuffer() noexcept;
	BodyState	_handleChunkedBody() noexcept;

	IoState		_receiveData() noexcept;
	IoState		_sendData() noexcept;

	IoState		_handleReceiveState( ssize_t read_bytes ) noexcept;
	IoState		_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;

	IoState		_getSocketState() const noexcept;

	void		_formResponse();

public:
	Connection() = default;
	Connection( Socket && socket );

	Connection( Connection const & ) = delete;
	Connection & operator=( Connection const & ) = delete;

	Connection( Connection && ) noexcept = default;
	Connection & operator=( Connection && ) noexcept = default;

	~Connection() = default;

	int		getFD() const noexcept;

	IoState	processEvents( uint32_t const events ) noexcept;

	int		getCGIPID() const noexcept;
	int		getCGIPipe( CGIOperation op );
	void	closeCGIPipe( CGIOperation op );
	bool	hasActiveCGI() const noexcept;

	EventAction	onCGIOutputReady();
	EventAction	onChildProcessExited( ChildExitInfo const & info );
};