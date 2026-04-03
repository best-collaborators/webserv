#include "CGIHandler.hpp"

CGIHandler::CGIHandler( CGIConfig & config ) : _max_body_size(config.max_body_size)
{
	CGIExecutor	executor(config);

	_pid = executor.getPID();
	_write_fd = PipeFD(executor.releaseWriteFD());
	_read_fd = PipeFD(executor.releaseReadFD());
}

CGIHandler::~CGIHandler()
{}

int CGIHandler::getPID() const noexcept
{
	return _pid;
}

int CGIHandler::getWriteFD() const noexcept
{
	return _write_fd.get();
}

int CGIHandler::getReadFD() const noexcept
{
	return _read_fd.get();
}

void CGIHandler::closeWritePipe() noexcept
{
	_write_fd.reset();
}

void CGIHandler::closeReadPipe() noexcept
{
	_read_fd.reset();
}

IoEvent CGIHandler::writeToCGI( std::string const & buffer ) noexcept
{
	ssize_t	remaining = buffer.length() - _write_offset;

	if (remaining <= 0)
	{
		_write_offset = 0;
		return IoEvent::Sent;
	}

	ssize_t	sent_bytes = write(_write_fd.get(), buffer.c_str() + _write_offset, remaining);

	if (sent_bytes == -1)
	{
		Log::warning("Write to CGI failed", "CGI");
		return IoEvent::Error;
	}

	_write_offset += sent_bytes;

	if (_write_offset >= buffer.length())
	{
		_write_offset = 0;
		return IoEvent::Sent;
	}

	return IoEvent::Pending;
}

IoEvent CGIHandler::readFromCGI() noexcept
{
	char	buffer_read[PIPE_BUFFER_SIZE];

	ssize_t read_bytes = read(_read_fd.get(), buffer_read, sizeof(buffer_read) - 1);

	if (read_bytes == 0)
	{
		Log::debug("CGI EOF reached", "CGI");
		return IoEvent::Done;
	}

	if (read_bytes == -1)
	{
		Log::warning("Read from CGI failed", "CGI");
		return IoEvent::Error;
	}

	_recv_buffer.append(buffer_read, read_bytes);

	if (!_headers_parsed)
	{
		size_t pos = _recv_buffer.find("\r\n\r\n");

		if (pos != std::string::npos)
		{
			_header_end_offset = pos + 4;
			_headers_parsed = true;
			std::string buffer_copy = _recv_buffer;

			Request _request;
			ParseContext parse_data = { .request = _request, .raw_bits = buffer_copy };
			HttpHeaderParser parser(parse_data, _max_body_size);
			parser.parse();

			_content_length = parse_data.request.get_content_length();

			if (parse_data.request.get_header_count(http::headers::CONTENT_LENGTH) && _content_length == 0)
			{
				_recv_buffer = _recv_buffer.substr(0, _header_end_offset);
				Log::debug("CGI content-length is 0, done", "CGI");
				return IoEvent::Done;
			}
		}
	}

	if (_headers_parsed && _content_length > 0)
	{
		size_t body_received = _recv_buffer.length() - _header_end_offset;

		if (body_received >= static_cast<size_t>(_content_length))
		{
			_recv_buffer = _recv_buffer.substr(0, _header_end_offset + _content_length);
			Log::debug("CGI body complete (content-length satisfied)", "CGI");
			return IoEvent::Done;
		}
	}
	return IoEvent::Pending;
}

std::string & CGIHandler::getBuffer() noexcept
{
	return _recv_buffer;
}

bool CGIHandler::isResponseReady() const noexcept
{
	return _is_output_ready && _is_child_dead;
}

EventAction CGIHandler::onChildProcessExited()
{
	_is_child_dead = true;

	if (_is_output_ready)
		return EventAction::EnableOutput;

	return EventAction::NoAction;
}

EventAction	CGIHandler::onCGIOutputReady()
{
	_is_output_ready = true;

	if (_is_child_dead)
		return EventAction::EnableOutput;

	return EventAction::NoAction;
}
