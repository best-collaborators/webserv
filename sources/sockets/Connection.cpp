#include "Connection.hpp"

Connection::Connection( Socket && socket ) : _fd(socket.getFD()), _stored_body_bytes(0), _socket(std::move(socket)), _cgi_pid(-1), _read_bytes(0), _sent_bytes(0)
{}

int Connection::getFD() const noexcept
{
	return _fd;
}

IoState Connection::processEvents( uint32_t const events ) noexcept
{
	if (events & EPOLLERR)
	{
		std::cout << "EPOLLERR" << std::endl;
		return IoState::Error;
	}

	if (events & EPOLLHUP)
	{
		std::cout << "EPOLLHUP" << std::endl;
		if (_cgi_handler)
			return IoState::CGIDone;

		return IoState::Closed;
	}

	if (events & EPOLLIN)
	{
		std::cout << "EPOLLIN" << std::endl;
		IoState state = _cgi_handler ? _cgi_handler->readFromCGI() : _receiveData();

		if (state != IoState::Pending)
			return state;
	}

	if (events & EPOLLOUT)
	{
		std::cout << "EPOLLOUT" << std::endl;

		_formResponse();

		IoState state = _cgi_handler ? _cgi_handler->writeToCGI(_read_buffer) : _sendData();

		if (state != IoState::Pending)
			return state;
	}

	return IoState::Pending;
}

void	Connection::_formResponse()
{
	if (_response_formed)
		return;

	if (_cgi_handler)
	{
		if (!_cgi_output_ready || _cgi_exit_status == CGIExitStatus::EMPTY)
			return;

		if (_cgi_exit_status == CGIExitStatus::SUCCESS)
		{
			std::cout << "CGI STATUS SUCCESS" << std::endl;
			_response.form_response(_request.get_status_code(), _request.copy_headers(), _cgi_handler->getBuffer());
		}
		else
		{
			std::cout << "CGI STATUS ERROR" << std::endl;
			_response.form_response(_request.get_status_code(), _request.copy_headers());
		}

		_resetCGIState();
	}
	else
	{
		_response.form_response(_request.get_status_code(), _request.copy_headers());
	}

	_removeBodyFromBuffer();
	_response_formed = true;
}

void	Connection::_resetCGIState()
{
	_cgi_handler.reset();
	_cgi_pid = -1;
	_cgi_exit_status = CGIExitStatus::EMPTY;
	_cgi_output_ready = false;
	_cgi_child_dead = false;
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

	_processHeader();

	// std::cout << "\n[io] read_buffer: " << "\n======" << is_header_received << "=========\n"
	// 	<< std::quoted(_read_buffer)
	// 	<< "\n===============\n";

	if (is_header_received && _processBody() == IoState::Pending)
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
					_cgi_pid = _cgi_handler->getPID();
				}
				catch(const std::exception& e)
				{
					std::cerr << "CGI EXECUTOR ERROR: " << e.what() << '\n';
					return IoState::Received; //! Return 500 error code and send response back
				}
				return IoState::CGIInit;
			}
		}
	}
	else if (!is_header_received)
		return IoState::Pending;

	return IoState::Received;
}

bool Connection::hasActiveCGI() const noexcept
{
	return _cgi_handler != nullptr;
}

EventAction Connection::onChildProcessExited( ChildExitInfo const & info )
{
	if (info.success())
		_cgi_exit_status = CGIExitStatus::SUCCESS;
	else
		_cgi_exit_status = CGIExitStatus::ERROR;

	_cgi_child_dead = true;

	if (_cgi_output_ready)
		return EventAction::EnableOutput;

	return EventAction::NoAction;
}

EventAction	Connection::onCGIOutputReady()
{
	_cgi_output_ready = true;

	if (_cgi_child_dead)
		return EventAction::EnableOutput;

	return EventAction::NoAction;
}

int Connection::getCGIPID() const noexcept
{
	return _cgi_pid;
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

bool Connection::_headersComplete() const noexcept
{
	return _read_buffer.find("\r\n\r\n") != std::string::npos;
}

void Connection::_parseHeaders() noexcept
{
	RequestParser header_parser(_request, _read_buffer);
	header_parser.parse_headers();

	std::cout << "Header received. Status code -> "
			<< _request.get_status_code() << std::endl;
}

void Connection::_consumeHeader() noexcept
{
	// size_t header_end_position = _read_buffer.find("\r\n\r\n");
	// _read_buffer.erase(0, header_end_position + 4);
}

HeaderState Connection::_handleHeaderMethod() noexcept
{
	std::string method = _request.get_header_value("method");
	if (!HttpMethod::hasBody(_request.get_method())) {

		if (_request.get_content_length() != -1 || _read_buffer.size() > 0) {
			std::cout << "[parser] Error (GET/OPTIONS/HEAD requests cannot have body)" << std::endl;
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

	// _request.print_http_request_values();

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
	if (_read_buffer.size() == 0 && _request.get_method() != HttpMethod::e_code::POST) {
		return BodyState::Complete;
	}

	if (_request.get_method() != HttpMethod::e_code::POST && _read_bytes != 0) {
		return BodyState::Invalid;
	}
	_stored_body_bytes = _read_buffer.size();
	std::cout << "\n[io] stored_body_bytes: " << _stored_body_bytes << "\n===============\n";
	if (_request.get_header_count("transfer-encoding")) {
		return BodyState::Chunked;
	}
	if (_stored_body_bytes == _request.get_content_length()) {
		return BodyState::Complete;
	}
	else if ( _request.get_header_count("content-length") && _stored_body_bytes > _request.get_content_length())
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

	RequestParser body_parser(_request, _read_buffer);
	body_parser.parse_body();

	std::cout << "Body received. Status code -> "
			  << _request.get_status_code() << std::endl;
}

BodyState Connection::_handleChunkedBody() noexcept
{
	std::cout << "[io] Request received (chunked)." << std::endl;

	std::cout << "_read_buffer is\n"
			<< _read_buffer << std::endl;

	RequestParser parser(_request, _read_buffer);
	parser.parse_body();

	std::cout << "Chunk received, chunk size is :"
			<< _request.get_current_chunk_size() << std::endl;

	std::cout << "Body is\n"
			<< _request.get_body() << std::endl;

	std::cout << "_read_buffer is\n"
			<< _read_buffer << std::endl;

	if (_request.is_chunk_received()) {

		std::cout << "Body received. Status code -> "
			  << _request.get_status_code() << std::endl;
		return BodyState::Complete;
	}

	return BodyState::Incomplete;
}

IoState Connection::_processBody() noexcept
{
	// std::cout << "body: " << _read_buffer << std::endl;

	switch (_checkBodyState())
	{
		case BodyState::Incomplete:
			std::cout << "[io] Request received (partial buffer)." << std::endl;
			return IoState::Pending;

		case BodyState::Complete:
			_handleCompleteBody();
			return IoState::Received;

		case BodyState::Chunked:
		{
			BodyState chunked_body_state = _handleChunkedBody();
			if (chunked_body_state == BodyState::Complete)
				return IoState::Received;
			return IoState::Pending;
		}

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

	_processHeader();
	if (is_header_received && _processBody() == IoState::Pending) {
		return IoState::Pending;
	}

	// !CGI

	_response.form_response(_request.get_status_code(), _request.copy_headers());
	_removeBodyFromBuffer();
	return IoState::Received;
}

IoState	Connection::_sendData() noexcept
{
	std::cout << "Connection::_sendData" << std::endl;
	std::cout << "\n[io] EPOLLOUT triggered for fd " << _fd << std::endl;

	std::cout << "[parser] Status code before response " << _request.get_status_code() << std::endl;

	is_header_received = false;
	_request.set_is_chunk_received(false);

	std::cout << "[io] send() starting..." << std::endl;

	_response.read_body_partially();

	size_t msg_len = _response.get_current_length();
	size_t total_msg_len = _response.get_total_response_length();
	const char *body = _response.get_body().c_str();

	// std::cout << "msg_len " << msg_len << std::endl;
	// std::cout << "total_msg_len " << total_msg_len << std::endl;

	std::cout << "==================RESPONSE==================\n"
		<< std::quoted(_response.get_body()) << std::endl
		<< "============================================\n";

	std::cout << "==================REQUEST==================\n"
		<< std::quoted(_read_buffer) << std::endl
		<< "============================================\n";

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
