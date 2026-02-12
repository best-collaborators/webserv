#include "Connection.hpp"

Connection::Connection( Socket && socket ) : _fd(socket.getFD()), _stored_body_bytes(0), _socket(std::move(socket)), _read_bytes(0), _sent_bytes(0)
{}

int Connection::getFD() const noexcept
{
	return _fd;
}

IoState Connection::processEvents( uint32_t const events ) noexcept
{
	if (events & EPOLLERR)
		return IoState::Error;

	if (events & EPOLLHUP)
	{
		if (_cgi_handler)
		{
			_response.form_response(_request.get_status_code(), _request.copy_headers(), _cgi_handler->getBuffer());
			_removeBodyFromBuffer();
			return IoState::Received;
		}
		return IoState::Closed;
	}

	if (events & EPOLLIN)
	{
		IoState state = _cgi_handler ? _cgi_handler->readFromCGI() : _receiveData();

		if (state == IoState::Received)
		{
			_response.form_response(_request.get_status_code(), _request.copy_headers());
			_removeBodyFromBuffer();
			return state;
		}

		if (state != IoState::Pending)
			return state;
	}

	if (events & EPOLLOUT)
	{
		IoState state = _cgi_handler ? _cgi_handler->writeToCGI(_read_buffer) : _sendData();

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

	_read_buffer.append(_recv_buffer, _read_bytes);
	_stored_bytes += _read_bytes;

	_processHeader();

	if (is_header_received == false)
		return IoState::Pending;

	if (_processBody() != IoState::Received)
		return IoState::Pending;

	if (_request.get_header_value("request-target") == "/cgi/test.js")
	{
		std::string const	contentLength = _request.get_header_value("content-length");
		
		if (!contentLength.empty())
		{
			std::size_t	pos {};
			const int i = std::stoi(contentLength, &pos);

			if (static_cast<ssize_t>(i) == _stored_body_bytes)
			{
				try
				{
					std::string	executable = "/usr/local/bin/node";
					std::string	scriptPath = "tests/test.js";
					std::vector<std::string> envVariables = { "TEST=Test!" };

					CGIConfig	config = {
						executable,
						scriptPath,
						envVariables
					};

					_cgi_handler = std::make_unique<CGIHandler>(config);
				}
				catch(const std::exception& e)
				{
					std::cerr << "CGI EXECUTOR ERROR: " << e.what() << '\n';
					return IoState::Received; //! Return 500 error code and send response back
				}
				return IoState::CGI;
			}
		}
	}

	return IoState::Received;
}

bool Connection::hasActiveCGI() const noexcept
{
	return _cgi_handler != nullptr;
}

int Connection::getCGIPipe( CGIOperation op )
{
	if (!_cgi_handler)
		return -1;	
	return op == CGIOperation::READ ? _cgi_handler->getReadFD() : _cgi_handler->getWriteFD();
}

void Connection::closeCGIPipe( CGIOperation op )
{
	if (!_cgi_handler)
		return;
	if (op == CGIOperation::READ)
	{
		_cgi_handler->closeReadPipe();
		_cgi_handler.reset();
	}
	else
		_cgi_handler->closeWritePipe();
}

bool Connection::_headersComplete() const noexcept
{
	return _read_buffer.find("\r\n\r\n") != std::string::npos;
}

void Connection::_parseHeaders() noexcept
{
	RequestParser request_parser(_request, _read_buffer);
	request_parser.parse_headers();

	std::cout << "Header received. Status code -> "
			<< _request.get_status_code() << std::endl;
}

void Connection::_consumeHeader() noexcept
{
	size_t header_end_position = _read_buffer.find("\r\n\r\n");
	_read_buffer.erase(0, header_end_position + 4);
}

HeaderState Connection::_handleHeaderMethod() noexcept
{
	std::string method = _request.get_header_value("method");
	if (method == "GET"	|| method == "OPTIONS" || method == "HEAD") {

		if (_request.get_content_length() != -1 || _read_buffer.size() > 0) {
			std::cout << "[parser] Error (GET/OPTIONS/HEAD requests cannot have body)";
			return HeaderState::Wrong;
		}
	}
	return HeaderState::Complete;
}

HeaderState Connection::_checkHeaderState() noexcept
{
	if (is_header_received) return HeaderState::Complete;

	if (!_headersComplete())
		return HeaderState::Incomplete;

	_parseHeaders();
	_consumeHeader();

	_request.print_http_request_values();

	is_header_received = true;
	return _handleHeaderMethod();
}

IoState Connection::_processHeader() noexcept
{
	switch (_checkHeaderState())
	{
		case HeaderState::Incomplete:
			std::cout << "[io] Request received (partial buffer)." << std::endl;
			return IoState::Pending;

		case HeaderState::Complete:
			return IoState::Received;

		//! CHECK RETURN STATUS CLOSE
		case HeaderState::Wrong:
			std::cout << "[io] Request received. Request header invalid." << std::endl;
			_request.set_status_code(404);
			return IoState::Received;

		default:
			return IoState::Received;
	}
	return IoState::Received;
}

BodyState Connection::_checkBodyState() noexcept
{
	if (_read_bytes == 0) {
		return BodyState::Complete;
	}
	if (_read_buffer.size() == 0 && _request.get_header_value("method") != "POST") {
		return BodyState::Complete;
	}
	if (_read_bytes != 0 && _request.get_header_value("method") != "POST") {
		return BodyState::Invalid;
	}
	_stored_body_bytes = _read_buffer.size();
	std::cout << "\n[io] stored_body_bytes: " << _stored_body_bytes << "\n===============\n";

	if (_stored_body_bytes == _request.get_content_length()) {
		return BodyState::Complete;
	}
	else if (_stored_body_bytes > _request.get_content_length())
	{
		std::cout << "Read buffer size" << _read_buffer.size() << std::endl;
		_request.set_status_code(404);
		return BodyState::Overflow;
	}
	return BodyState::Incomplete;
}

void Connection::_handleCompleteBody() noexcept
{
	std::cout << "[io] Request received (complete)." << std::endl;

	RequestParser parser(_request, _read_buffer);
	parser.parse_body();

	_request.set_status_code(parser.get_status_code());

	std::cout << "Body received. Status code -> "
			  << _request.get_status_code() << std::endl;
}

IoState Connection::_processBody() noexcept
{
	switch (_checkBodyState())
	{
		case BodyState::Incomplete:
			std::cout << "[io] Request received (partial buffer)." << std::endl;
			return IoState::Pending;

		case BodyState::Complete:
			_handleCompleteBody();
			return IoState::Received;

		//! CHECK RETURN STATUS CLOSE
		case BodyState::Overflow:
			std::cout << "[io] Request received. Body too long." << std::endl;
			_request.set_status_code(404);
			return IoState::Received;

		case BodyState::Invalid:
			std::cout << "[io] Request received. Request is not suppose to have body." << std::endl;
			_request.set_status_code(404);
			return IoState::Received;
	}
	return IoState::Received;
}

void	Connection::_removeBodyFromBuffer() noexcept
{
	_read_buffer.erase(0, _request.get_content_length());
}

IoState	Connection::_saveToBuffer() noexcept
{
	_read_buffer.append(_recv_buffer, _read_bytes);
	_stored_bytes += _read_bytes;

	// std::cout << "\n[io] read_bytes: " << _read_bytes
	// 	<< "\n===============\n";
	// 	std::cout << "connection fd " << _socket.getFD()
	// 	<< "\n=================\n"
	// 	<< _read_buffer.substr(0, _stored_bytes)
	// 	<< "=================\n";

	_processHeader();
	if (_processBody() == IoState::Pending) {
		return IoState::Pending;
	}

	// !CGI
	
	_response.form_response(_request.get_status_code(), _request.copy_headers());
	_removeBodyFromBuffer();
	return IoState::Received;
}

IoState	Connection::_sendData() noexcept
{
	int	fd = _socket.getFD();
	std::cout << "\n[io] EPOLLOUT triggered for fd " << fd << std::endl;

	std::cout << "[parser] Status code before response " << _request.get_status_code() << std::endl;

	is_header_received = false;

	std::cout << "[io] send() starting..." << std::endl;

	size_t msg_len = _response.get_current_length();
	size_t total_msg_len = _response.get_total_response_length();
	const char *body = _response.get_body().c_str();

	std::cout << "==================RESPONSE==================\n"
		<< body << std::endl
		<< "============================================\n";

	std::cout << "==================REQUEST==================\n"
		<< _read_buffer << std::endl
		<< "============================================\n";

	ssize_t curr_sent_bytes = send(fd, body, msg_len, 0);

	_response.consume_body(curr_sent_bytes);
	_sent_bytes += curr_sent_bytes;

	// std::cout << "send bytes " << _sent_bytes << std::endl;
	// _response.set_response_length(msg_len - curr_sent_bytes);
	return _handleSendState(curr_sent_bytes, total_msg_len);
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
