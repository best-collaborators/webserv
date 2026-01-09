#include "Connection.hpp"

Connection::Connection() : _fd(-1), _read_bytes(0), _stored_bytes(0)
{}

Connection::Connection( int fd ) : _fd(fd), _read_bytes(0), _stored_bytes(0)
{}

Connection::~Connection()
{}

IoState	Connection::_saveToBuffer() noexcept
{
	if (_read_bytes <= READ_BUFFER_SIZE)
	{
		std::cout << "\n[io] read_bytes: " << _read_bytes << "\n===============\n";
		
		_read_buffer.append(_temp_buffer, _read_bytes);
		_stored_bytes += _read_bytes;
		std::cout << "connection fd " << _fd << "\n=================\n" << _read_buffer.substr(0, _stored_bytes) << "===============" << std::endl;

		if (_read_bytes < READ_BUFFER_SIZE)
		{
			std::cout << "[io] Request received (complete)." << std::endl;
			return IoState::Ready;
		}
		else if (_read_bytes == READ_BUFFER_SIZE)
		{
			std::cout << "[io] Request received (partial buffer)." << std::endl;
			return IoState::Pending;
		}
	}

	return IoState::Pending;
}

IoState	Connection::_checkSocketHealth() noexcept
{
	int			error = 0;
	socklen_t	len = sizeof(error);

	if (!getsockopt(_fd, SOL_SOCKET, SO_ERROR, &error, &len))
	{
		if (error == 0)
		{
			return IoState::Pending;
		}
	}

	return IoState::Error;
}

IoState	Connection::receiveData() noexcept
{
	std::cout << "\n[io] EPOLLIN triggered on fd " << _fd << std::endl;
	std::cout << "[io] recv() starting..." << std::endl;

	_read_bytes = recv(_fd, _temp_buffer, sizeof(_temp_buffer), 0);

	std::cout << "[io] recv() completed." << std::endl;

	if (_read_bytes == 0)
	{
		std::cout << "[io] Peer closed fd " << _fd << "." << std::endl;

		return IoState::Closed;
	}
	else if (_read_bytes < 0)
	{
		return _checkSocketHealth();
	}
	else if (_read_bytes > 0)
	{
		return _saveToBuffer();
	}

	return IoState::Pending;
}

IoState	Connection::sendData() noexcept
{
	std::cout << "\n[io] EPOLLOUT triggered for fd " << _fd << std::endl;

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

	ssize_t sent_bytes = send(_fd, message.c_str(), message.length(), 0);

	if (sent_bytes <= 0)
	{
		std::cout << "[io] Send failed or would block." << std::endl;
		return _checkSocketHealth();
	}
	else if (sent_bytes < message_len)
	{
		std::cout << "[io] Response sent partially." << std::endl;
		return IoState::Pending;
	}
	else if (sent_bytes == message_len)
	{
		std::cout << "[io] Response sent (complete)." << std::endl;

		return IoState::Ready;
	}

	return IoState::Pending;
}
