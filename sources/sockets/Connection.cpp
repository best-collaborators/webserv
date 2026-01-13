#include "Connection.hpp"

Connection::Connection( Socket && socket ) : _socket(std::move(socket)), _read_bytes(0), _stored_bytes(0)
{}

IoState Connection::processEvents( uint32_t const events ) noexcept
{
	if (events & EPOLLERR)
		return IoState::Error;

	if (events & EPOLLHUP)
		return IoState::Closed;

	if (events & EPOLLIN)
	{
		IoState state = _receiveData();

		if (state != IoState::Pending)
			return state;
	}

	if (events & EPOLLOUT)
	{
		IoState state = _sendData();

		if (state != IoState::Pending)
			return state;
	}

	return IoState::Pending;
}

IoState Connection::_receiveData() noexcept
{
	std::cout << "\n[io] EPOLLIN triggered on fd " << _socket.getFD() << std::endl;
	std::cout << "[io] recv() starting..." << std::endl;

	_read_bytes = recv(_socket.getFD(), _recv_buffer, sizeof(_recv_buffer), 0);

	std::cout << "[io] recv() completed." << std::endl;

	return _handleReceiveState(_read_bytes);
}

IoState	Connection::_handleReceiveState( ssize_t read_bytes ) noexcept
{
	if (read_bytes < 0)
	{
		return _getSocketState();
	}
	else if (read_bytes == 0)
	{
		std::cout << "[io] Peer closed fd " << _socket.getFD() << "." << std::endl;

		return IoState::Closed;
	}

	return _saveToBuffer();
}

IoState	Connection::_saveToBuffer() noexcept
{
	std::cout << "\n[io] read_bytes: " << _read_bytes << "\n===============\n";
	
	_read_buffer.append(_recv_buffer, _read_bytes);
	_stored_bytes += _read_bytes;

	std::cout << "connection fd " << _socket.getFD() << "\n=================\n" << _read_buffer.substr(0, _stored_bytes) << "===============" << std::endl;

	if (_read_bytes < READ_BUFFER_SIZE)
	{
		std::cout << "[io] Request received (complete)." << std::endl;
		return IoState::Received;
	}
	else if (_read_bytes == READ_BUFFER_SIZE)
	{
		std::cout << "[io] Request received (partial buffer)." << std::endl;
	}

	return IoState::Pending;
}

IoState	Connection::_sendData() noexcept
{
	int	fd = _socket.getFD();
	std::cout << "\n[io] EPOLLOUT triggered for fd " << fd << std::endl;

	std::string body =
		"<html>\n"
		"<head><title>200 OK</title></head>\n"
		"<body>\n"
		"<center><h1>200 OK</h1></center>\n"
		"</body>\n"
		"</html>\n";

	std::string headers =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " + std::to_string(body.size()) + "\r\n"
		"Connection: keep-alive\r\n"
		"\r\n";

	std::string message = headers + body;

	ssize_t	message_len = message.length();

	std::cout << "[io] send() starting..." << std::endl;

	ssize_t sent_bytes = send(fd, message.c_str(), message_len, 0);

	return _handleSendState(sent_bytes, message_len);
}

IoState	Connection::_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept
{
	if (sent_bytes < 0)
	{
		std::cout << "[io] Send failed" << std::endl;
		return _getSocketState();
	}
	else if (sent_bytes == message_length)
	{
		std::cout << "[io] Response sent (complete)." << std::endl;
		return IoState::Sent;
	}
	else if (sent_bytes < message_length) //! Implement partial send
	{
		std::cout << "[io] Response sent partially." << std::endl;
	}

	return IoState::Pending;
}

IoState	Connection::_getSocketState() const noexcept
{
	if (_socket.isHealthy() == false)
	{
		return IoState::Error;
	}

	return IoState::Pending;
}
