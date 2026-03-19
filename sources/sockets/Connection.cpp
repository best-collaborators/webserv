#include "Connection.hpp"
#include <sstream>
#include <cctype>

Connection::Connection( ServerBlock const * server_block, Socket && socket ) : _last_activity(std::chrono::steady_clock::now()), _fd(socket.getFD()), _socket(std::move(socket)), _server_block(server_block), _request_reader(server_block), _sent_bytes(0), _read_bytes(0)
{
	(void) _server_block;
}

int Connection::getFD() const noexcept
{
	return _fd;
}

void Connection::abortCGI() noexcept
{
	_request_reader.setStatusCode(HttpStatus::e_code::GATEWAY_TIMEOUT);
	_cgi_handler.reset();
	_cgi_start_time.reset();
}

void Connection::abortCGIWithError() noexcept
{
	_request_reader.setStatusCode(HttpStatus::e_code::INTERNAL_SERVER_ERROR);
	_cgi_handler.reset();
	_cgi_start_time.reset();
}

bool Connection::headersSentToClient() const noexcept
{
	return _headers_sent_to_client;
}

void	Connection::_formResponse()
{
	if (_response_formed)
		return;

	if (_cgi_handler)
		_formCGIResponse();
	else 
		_response_writer.formResponse(_request_reader.request());

	size_t content_length = _request_reader.getContentLength();
	_buffer_manager.consume(content_length);
	_response_formed = true;
	_request_reader.reset();
}

void	Connection::_formCGIResponse()
{
	if (!_cgi_handler->isResponseReady())
		return;

	std::string & cgi_buffer = _cgi_handler->getBuffer();

	Request request;
	ParseContext context = { .request = request, .raw_bits = cgi_buffer};
	CGIValidator cgi_validator(context);
	HttpStatus::e_code cgi_status = cgi_validator._validateCGIOutput();

	if (cgi_status != HttpStatus::e_code::OK)
	{
		Log::critical("CGI returned error: " + cgi_buffer, "cgi");
		_request_reader.setStatusCode(cgi_status);
		_response_writer.formResponse(_request_reader.request());
	}
	else
	{
		_response_writer.formResponse(_request_reader.request(), cgi_buffer, true);
	}

	_cgi_handler.reset();
	_cgi_start_time.reset();
}

IoResult	Connection::processConnectionEvents( uint32_t const events )
{
	if (events & EPOLLERR)
	{
		Log::debug("EPOLLERR", "Connection");
		return { IoSource::Connection, IoEvent::Error };
	}

	if (events & EPOLLHUP)
	{
		Log::debug("EPOLLHUP", "Connection");
		return { IoSource::Connection, IoEvent::Closed };
	}

	if (events & EPOLLIN)
	{
		Log::debug("EPOLLIN", "Connection");
		IoEvent state = _receiveData();

		if (state == IoEvent::Init)
		{
			_cgi_start_time = std::chrono::steady_clock::now();
			Log::debug("CGI process started", "CGI");
			return { IoSource::CGI, IoEvent::Init };
		}
		if (state != IoEvent::Pending)
			return { IoSource::Connection, state };
	}

	if (events & EPOLLOUT)
	{
		Log::debug("EPOLLOUT", "Connection");

		_formResponse();

		IoEvent state = _sendData();

		if (state != IoEvent::Pending)
			return { IoSource::Connection, state };
	}

	return { IoSource::Connection, IoEvent::Pending };
}

void Connection::resetLastActivity() noexcept
{
	_last_activity = std::chrono::steady_clock::now();
}

std::chrono::time_point<std::chrono::steady_clock> Connection::getLastActivity() const noexcept
{
	return _last_activity;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>> Connection::getCGIStartTime() const noexcept
{
	return _cgi_start_time;
}

IoResult Connection::processCGIEvents( uint32_t const events )
{
	if (events & EPOLLERR)
	{
		Log::debug("EPOLLERR", "CGI");
		return { IoSource::CGI, IoEvent::Error };
	}

	if (events & EPOLLIN)
	{
		Log::debug("EPOLLIN", "CGI");
		IoEvent state = _cgi_handler->readFromCGI();

		if (state == IoEvent::Done || state == IoEvent::Error)
			return { IoSource::CGI, state };
	}

	if (events & EPOLLHUP)
	{
		Log::debug("EPOLLHUP on CGI pipe", "CGI");

		IoEvent state = _cgi_handler->readFromCGI();

		if (state == IoEvent::Error)
		{
			_cgi_start_time.reset();
			return { IoSource::CGI, IoEvent::Error };
		}
		if (state == IoEvent::Done)
		{
			_cgi_start_time.reset();
			return { IoSource::CGI, IoEvent::Done };
		}
		return { IoSource::CGI, IoEvent::Pending };
	}

	if (events & EPOLLOUT)
	{
		Log::debug("EPOLLOUT", "CGI");
		IoEvent state = _cgi_handler->writeToCGI(_buffer_manager.getBuffer());

		if (state != IoEvent::Pending)
			return { IoSource::CGI, state };
	}

	return { IoSource::CGI, IoEvent::Pending };
}

IoEvent Connection::_receiveData() noexcept
{
	Log::debug("[io] EPOLLIN triggered on fd " + std::to_string(_fd), "Connection");
	Log::debug("recv() starting", "Connection");

	_read_bytes = recv(
		_fd,
		_buffer_manager.getRecvBuffer(),
		_buffer_manager.getReceiveBufferSize(),
		0
	);

	Log::debug("recv() completed", "Connection");

	return _handleReceiveState(_read_bytes);
}

IoEvent Connection::_tryInitCGI() noexcept
{
	auto headers = _request_reader.getHeaders();

	File const & file = _request_reader.getFile();

	try
	{
		CGIConfig	config = cgi::buildConfig(headers, file);
		_cgi_handler.emplace(config);
	}
	catch(const std::exception& e)
	{
		Log::error("CGI executor error " + std::to_string(*(e.what())), "CGI");
		_request_reader.setStatusCode(HttpStatus::e_code::SERVICE_UNAVAILABLE);
		return IoEvent::Received;
	}
	return IoEvent::Init;
}

IoEvent	Connection::_handleReceiveState( ssize_t read_bytes ) noexcept
{
	Log::info("Peer closed fd " + std::to_string(_fd), "Connection");
	if (read_bytes < 0)
	{
		return _getSocketState();
	}
	else if (read_bytes == 0)
	{
		return IoEvent::Closed;
	}

	_buffer_manager.append(_read_bytes);
	ReaderState reader_state = _request_reader.read(_buffer_manager.getBuffer(), _read_bytes);

	resetLastActivity();

	switch (reader_state)
	{
	case CGI:
		return _tryInitCGI();

	case AwaitingHeaders:
	case AwaitingBody:
		return IoEvent::Pending;

	case Complete:
	case Error:
		return IoEvent::Received;
	
	default:
		break;
	}

	return IoEvent::Received;
}

EventAction Connection::onChildProcessExited()
{
	if (_cgi_handler)
		return _cgi_handler->onChildProcessExited();

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

IoEvent	Connection::_sendData() noexcept
{
	Log::debug("EPOLLOUT triggered for fd " + std::to_string(_fd), "Connection");
	Log::debug("Start sending data...", "Connection");

	_response_writer.write(_request_reader.request());

	size_t msg_len = _response_writer.currResponseLength();
	size_t total_msg_len = _response_writer.totalLength();
	const char *body =  _response_writer.getResponseData();

	ssize_t curr_sent_bytes = send(_fd, body, msg_len, 0);

	if (curr_sent_bytes > 0)
	{
		_headers_sent_to_client = true;
		_response_writer.consume(curr_sent_bytes);
		_sent_bytes += curr_sent_bytes;
	}

	return _handleSendState(_sent_bytes, total_msg_len);
}

IoEvent	Connection::_handleSendState( ssize_t sent_bytes, ssize_t message_length ) noexcept
{
	if (sent_bytes < 0)
	{
		Log::error("Send failed", "Connection");
		return _getSocketState();
	}

	resetLastActivity();

	if (sent_bytes == message_length)
	{
		Log::debug("Response sent (complete)", "Connection");
		_response_formed = false;
		_headers_sent_to_client = false;
		_sent_bytes = 0;
		_request_reader.reset();
		return IoEvent::Sent;
	}
	else if (sent_bytes < message_length)
	{
		Log::debug("Response sent partially", "Connection");
	}

	return IoEvent::Pending;
}

IoEvent	Connection::_getSocketState() const noexcept
{
	if (_socket.isHealthy() == false)
	{
		return IoEvent::Error;
	}

	return IoEvent::Pending;
}
