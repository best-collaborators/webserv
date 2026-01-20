#include "Connection.hpp"

Connection::Connection( Socket && socket ) : _content_length(0), _stored_body_bytes(0),  _socket(std::move(socket)), _read_bytes(0), _stored_bytes(0)
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

	//CHECK FOR CONTENT LENGTH IF _read_bytes > _content_length ====> _read_buffer.append(_recv_buffer, _content_length);
	_read_buffer.append(_recv_buffer, _read_bytes);
	_stored_bytes += _read_bytes;
	// std::cout << "connection fd " << _socket.getFD() << "\n=================\n" << _read_buffer.substr(0, _stored_bytes) << "===============" << std::endl;

	
	if (!is_header_received) {

		size_t header_end_position = _read_buffer.find("\r\n\r\n");
		if (header_end_position != std::string::npos)
		{
			requestParser.parse_headers(_read_buffer);
			_parse_result = requestParser.create_request_parse_result();

			std::cout << "Header received. Status code -> " << _parse_result.get_status_code() << std::endl;
			requestParser.print_http_request_values();

			_read_buffer.erase(0, header_end_position + 4);

			if (_parse_result.get_method() == "GET"
				|| _parse_result.get_method() == "OPTIONS" 
				|| _parse_result.get_method() == "HEAD") {
				std::cout << "[io] Request received (complete)." << std::endl;
				return IoState::Received;
			}
			else if (_parse_result.get_method() == "POST")
			{
				_content_length = _parse_result.get_content_length();
			}
			is_header_received = true;
		}
	}

	if (is_header_received)
	{
		_stored_body_bytes = _read_buffer.size();
		std::cout << "\n[io] stored_body_bytes: " << _stored_body_bytes << "\n===============\n";
		if (_stored_body_bytes >= _content_length)
		{
			std::cout << "[io] Request received (complete)." << std::endl;

			requestParser.parse_body(_read_buffer);
			_parse_result = requestParser.create_request_parse_result();
			std::cout << "Body received. Status code -> " << _parse_result.get_status_code() << std::endl;
			return IoState::Received;
		}
	}
	std::cout << "[io] Request received (partial buffer)." << std::endl;
	return IoState::Pending;
}

IoState	Connection::_sendData() noexcept
{
	int	fd = _socket.getFD();
	std::cout << "\n[io] EPOLLOUT triggered for fd " << fd << std::endl;

	std::cout << "Before response. Status code -> " << _parse_result.get_status_code() << std::endl;
	std::string message = _response.form_reponse(_parse_result);
	std::cout << "==================RESPONSE==================\n" 
	<< message << "\n" << "============================================\n";

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
