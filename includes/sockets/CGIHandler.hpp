#pragma once

#include "PipeFD.hpp"
#include "IoState.hpp"
#include "CGIConfig.hpp"
#include "CGIExecutor.hpp"

class CGIHandler
{
private:
	PipeFD		_write_fd;
	PipeFD		_read_fd;

	std::string	_recv_buffer;

public:
	CGIHandler() = default;
	CGIHandler( CGIConfig & config );
	~CGIHandler();

	int			getWriteFD() const noexcept;
	int			getReadFD() const noexcept;

	void		closeWritePipe() noexcept;
	void		closeReadPipe() noexcept;

	IoState		writeToCGI( std::string const & buffer ) noexcept;
	IoState		readFromCGI() noexcept;

	std::string & getBuffer() noexcept;
};