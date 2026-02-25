#include "CGIHandler.hpp"

CGIHandler::CGIHandler( CGIConfig & config ) : _content_length(-1)
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

CGIExitStatus CGIHandler::getExitStatus() const noexcept
{
	return _exit_status;
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
	std::cout << "writeToCGI function" << std::endl;

	ssize_t	buffer_len = buffer.length();
	ssize_t	sent_bytes = write(_write_fd.get(), buffer.c_str(), buffer_len);

	if (sent_bytes == buffer_len)
		return IoEvent::Sent;
	else if (sent_bytes == -1)
	{
		std::cerr << "[CGI] (CGIHandler::writeToCGI) write to CGI failed" << std::endl;
		return IoEvent::Error;
	}

	return IoEvent::Pending;
}

IoEvent CGIHandler::readFromCGI() noexcept
{
	std::cout << "readFromCGI function" << std::endl;

	char	buffer_read[65537];

	ssize_t read_bytes = 0;
	read_bytes = read(_read_fd.get(), buffer_read, sizeof(buffer_read));
	if (read_bytes > 0)
	{
		buffer_read[read_bytes] = '\0';
		_recv_buffer.append(buffer_read, read_bytes);

		size_t pos = _recv_buffer.find("\r\n\r\n");

		if (pos != std::string::npos)
		{
			std::string copy = _recv_buffer;

			Request _request;
			ParseContext parse_data = { .request = _request, .raw_bits = _recv_buffer };
			HttpHeaderParser parser(parse_data);
			parser.parse();

			_content_length = parse_data.request.get_content_length();

			if (_content_length == 0)
				return IoEvent::Done;
		}
		else
		{
			/* 
				! If on N iteration pos not found -> invalid structure of responses, headers is required in return of CGI

				Treat it as invalid CGI output
				Return 500 Internal Server Error

				At least should be present:

				Content-Type: text/html

				<html>...</html>
			*/
		}

		if (_content_length != -1)
		{
			if (static_cast<ssize_t>(_recv_buffer.length()) >= _content_length)
			{
				_recv_buffer = _recv_buffer.substr(0, _content_length);
				return IoEvent::Done;
			}
		}
	}
	else if (read_bytes == 0)
	{
		return IoEvent::Done;
	}
	else if (read_bytes == -1)
	{
		std::cerr << "[CGI] (CGIHandler::readFromCGI) read from CGI failed" << std::endl;
		return IoEvent::Error;
	}

	return IoEvent::Pending;
}

std::string & CGIHandler::getBuffer() noexcept
{
	return _recv_buffer;
}

bool CGIHandler::isResponseReady() const noexcept
{
	if (_is_output_ready && _is_child_dead && _exit_status != CGIExitStatus::EMPTY)
		return true;

	return false;
}

EventAction CGIHandler::onChildProcessExited( ChildExitInfo const & info )
{
	if (info.success())
		_exit_status = CGIExitStatus::SUCCESS;
	else
		_exit_status = CGIExitStatus::ERROR;

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
