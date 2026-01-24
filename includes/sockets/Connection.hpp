#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "IoState.hpp"
#include "Socket.hpp"

#include <unordered_map>

#include "Response.hpp"
#include "RequestParser.hpp"
#include "BodyState.hpp"
#include "HeaderState.hpp"

class Connection
{
private:
	static constexpr int	READ_BUFFER_SIZE = 32768;

	bool			is_header_received = false;
	ssize_t			_stored_body_bytes;
	HttpResponse	_response;
	HttpRequest		_request;

	Socket		_socket;

	char		_recv_buffer[READ_BUFFER_SIZE];
	ssize_t		_read_bytes;
	ssize_t		_stored_bytes;
	std::string	_read_buffer;

	BodyState	_check_body_state() noexcept;
	void		_handle_complete_body() noexcept;
	IoState		_process_body() noexcept;

	void		_handle_received_header() noexcept;
	bool		_headers_complete() const noexcept;
	void		_parse_headers() noexcept;
	void		_consume_header() noexcept;
	HeaderState	_handle_header_method() noexcept;
	HeaderState	_proceed_header() noexcept;

	IoState		_receiveData() noexcept;
	IoState		_sendData() noexcept;

	IoState		_handleReceiveState( ssize_t read_bytes ) noexcept;
	IoState		_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept;

	IoState		_saveToBuffer() noexcept;

	IoState		_getSocketState() const noexcept;

public:
	Connection() = default;
	Connection( Socket && socket );

	Connection( Connection const & ) = delete;
	Connection & operator=( Connection const & ) = delete;

	Connection( Connection && ) noexcept = default;
	Connection & operator=( Connection && ) noexcept = default;

	~Connection() = default;

	IoState	processEvents( uint32_t const events ) noexcept;
};