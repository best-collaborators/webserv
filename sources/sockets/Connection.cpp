#include "Connection.hpp"

Connection::Connection( Socket && socket ) : _fd(socket.getFD()), _socket(std::move(socket)), _read_bytes(0), _sent_bytes(0)
{}

int Connection::getFD() const noexcept
{
	return _fd;
}

IoEvent toIoEvent(IoState state)
{
	switch (state)
	{
	case IoState::Pending:
		return IoEvent::Pending;
	case IoState::Closed:
		return IoEvent::Closed;
	case IoState::Error:
		return IoEvent::Error;
	case IoState::Sent:
		return IoEvent::Sent;
	case IoState::Received:
		return IoEvent::Received;
	case IoState::Init:
		return IoEvent::Init;
	}
	return IoEvent::Error;
}

IoResult	Connection::processConnectionEvents( uint32_t const events )
{
	if (events & EPOLLERR)
	{
		std::cout << "EPOLLERR" << std::endl;
		return { IoSource::Connection, IoEvent::Error };
	}

	if (events & EPOLLHUP)
	{
		std::cout << "EPOLLHUP" << std::endl;
		return { IoSource::Connection, IoEvent::Closed };
	}

	if (events & EPOLLIN)
	{
		std::cout << "EPOLLIN" << std::endl;
		IoState state = _receiveData();

		if (state == IoState::Init)
			return { IoSource::CGI, IoEvent::Init };
		if (state != IoState::Pending)
			return { IoSource::Connection, toIoEvent(state) };
	}

	if (events & EPOLLOUT)
	{
		std::cout << "EPOLLOUT" << std::endl;

		_response_writer.formResponse();

		IoState state = _sendData();

		if (state != IoState::Pending)
			return { IoSource::Connection, toIoEvent(state) };
	}

	return { IoSource::Connection, IoEvent::Pending };
}

IoResult Connection::processCGIEvents( uint32_t const events )
{
	if (events & EPOLLERR)
	{
		std::cout << "EPOLLERR" << std::endl;
		return { IoSource::CGI, IoEvent::Error };
	}

	if (events & EPOLLHUP)
	{
		std::cout << "EPOLLHUP" << std::endl;
		return { IoSource::CGI, IoEvent::Done };
	}

	if (events & EPOLLIN)
	{
		std::cout << "EPOLLIN" << std::endl;
		IoState state = _cgi_handler->readFromCGI();

		if (state != IoState::Pending)
			return { IoSource::CGI, toIoEvent(state) };
	}

	if (events & EPOLLOUT)
	{
		std::cout << "EPOLLOUT" << std::endl;
		IoState state = _cgi_handler->writeToCGI(_buffer_manager.getBuffer());

		if (state != IoState::Pending)
			return { IoSource::CGI, toIoEvent(state) };
	}

	return { IoSource::CGI, IoEvent::Pending };
}

IoState Connection::_receiveData() noexcept
{
	std::cout << "\n[io] EPOLLIN triggered on fd " << _socket.getFD() << std::endl;
	std::cout << "[io] recv() starting..." << std::endl;

	_read_bytes = recv(
		_socket.getFD(),
		_buffer_manager.getRecvBuffer(),
		_buffer_manager.getReceiveBufferSize(),
		0
	);

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

	_buffer_manager.append(_read_bytes);
	ReaderState reader_state = _request_reader.read(_buffer_manager.getBuffer(), _read_bytes);
	switch (reader_state)
	{
	case CGI:
		if (_request_reader.request().get_header_value("request-target") == "/cgi/test.js")
		{
			std::string const	contentLength = _request_reader.request().get_header_value("content-length");

			if (!contentLength.empty())
			{
				std::size_t	pos {};
				const int i = std::stoi(contentLength, &pos);

				if (static_cast<ssize_t>(i) == _request_reader.getStoredBodyBytes())
				{
					try
					{
						std::string	executable = "/home/rmzvr/.nvm/versions/node/v24.11.1/bin/node";
						std::string	scriptPath = "tests/test.js";
						std::vector<std::string> envVariables = { "TEST=Test!" };

						CGIConfig	config = {
							executable,
							scriptPath,
							envVariables
						};

						_cgi_handler.emplace(config);
					}
					catch(const std::exception& e)
					{
						std::cerr << "CGI EXECUTOR ERROR: " << e.what() << '\n';
						return IoState::Received; //! Return 500 error code and send response back
					}
					return IoState::Init;
				}
			}
		}
		break;

	case AwaitingHeaders:
	case AwaitingBody:
		return IoState::Pending;

	case Complete:
	case Error:
		return IoState::Received;
	
	default:
		break;
	}



	return IoState::Received;
}

EventAction Connection::onChildProcessExited( ChildExitInfo const & info )
{
	if (_cgi_handler)
		return _cgi_handler->onChildProcessExited(info);

	return EventAction::NoAction;
}

EventAction	Connection::onCGIOutputReady()
{
	if (_cgi_handler)
		return _cgi_handler->onCGIOutputReady();

	return EventAction::NoAction;
}

int Connection::getCGIPID() const noexcept
{
	if (!_cgi_handler)
		return -1;

	return _cgi_handler->getPID();
}

int Connection::getCGIPipe(CGIOperation op)
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
		_cgi_handler->closeReadPipe();
	else
		_cgi_handler->closeWritePipe();
}

IoState	Connection::_sendData() noexcept
{
	std::cout << "Connection::_sendData" << std::endl;
	std::cout << "\n[io] EPOLLOUT triggered for fd " << _fd << std::endl;

	// std::cout << "[parser] Status code before response " << _request.get_status_code() << std::endl;
	_request_reader.reset();

	std::cout << "[io] send() starting..." << std::endl;

	_response.read_body_partially();

	size_t msg_len = _response.get_current_length();
	size_t total_msg_len = _response.get_total_response_length();
	const char *body = _response.get_body().c_str();

	// std::cout << "msg_len " << msg_len << std::endl;
	// std::cout << "total_msg_len " << total_msg_len << std::endl;

	// std::cout << "==================RESPONSE==================\n"
	// 	<< std::quoted(_response.get_body()) << std::endl
	// 	<< "============================================\n";

	// std::cout << "==================REQUEST==================\n"
	// 	<< std::quoted(_read_buffer) << std::endl
	// 	<< "============================================\n";

	ssize_t curr_sent_bytes = send(_fd, body, msg_len, 0);

	_response.consume_body(curr_sent_bytes);
	_sent_bytes += curr_sent_bytes;

	// std::cout << "curr send bytes " << curr_sent_bytes << std::endl;
	// std::cout << "send bytes " << _sent_bytes << std::endl;
	// std::cout << "body ==>" << _response.get_body() << std::endl;
	return _handleSendState(_sent_bytes, total_msg_len);
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
		_response_formed = false;
		_sent_bytes = 0;
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
