#pragma once

#include "Logger.hpp"
#include "PipeFD.hpp"
#include "IoResult.hpp"
#include "CGIConfig.hpp"
#include "CGIExecutor.hpp"
#include "CGIExitStatus.hpp"
#include "ChildExitInfo.hpp"
#include "EventAction.hpp"
#include "ParseContext.hpp"
#include "HttpHeaderParser.hpp"

class CGIHandler
{
private:
	int				_pid;
	PipeFD			_write_fd;
	PipeFD			_read_fd;

	ssize_t			_content_length;
	bool			_is_output_ready = false;
	bool			_is_child_dead = false;
	CGIExitStatus	_exit_status = CGIExitStatus::EMPTY;

	std::string		_recv_buffer;

public:
	CGIHandler() = default;
	CGIHandler( CGIConfig & config );

	CGIHandler( CGIConfig const & ) = delete;
	CGIHandler & operator=( CGIHandler const & ) = delete;

	CGIHandler( CGIHandler && ) noexcept = default;
	CGIHandler & operator=( CGIHandler && ) noexcept = default;

	~CGIHandler();

	int				getPID() const noexcept;
	int				getWriteFD() const noexcept;
	int				getReadFD() const noexcept;
	CGIExitStatus	getExitStatus() const noexcept;

	void			closeWritePipe() noexcept;
	void			closeReadPipe() noexcept;

	IoEvent			writeToCGI( std::string const & buffer ) noexcept;
	IoEvent			readFromCGI() noexcept;

	std::string &	getBuffer() noexcept;

	bool			isResponseReady() const noexcept;

	EventAction		onCGIOutputReady();
	EventAction		onChildProcessExited( ChildExitInfo const & info );
};