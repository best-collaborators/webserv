#pragma once

#include <iostream>
#include <memory>
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
#include "ChildExitInfo.hpp"
#include "CGIExitStatus.hpp"

enum class EventAction : short
{
	NoAction,
	EnableOutput
};

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
	int			_cgi_pid;
	std::unique_ptr<CGIHandler> _cgi_handler;

	bool		_cgi_output_ready = false;
	bool		_cgi_child_dead = false;
	CGIExitStatus	_cgi_exit_status = CGIExitStatus::EMPTY;
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

	IoState		_receiveData() noexcept;
	IoState		_sendData() noexcept;

	IoState		_handleReceiveState( ssize_t read_bytes ) noexcept;
	IoState		_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;

	IoState		_saveToBuffer() noexcept;

	IoState		_getSocketState() const noexcept;

	void		_formResponse();
	void		_resetCGIState();

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