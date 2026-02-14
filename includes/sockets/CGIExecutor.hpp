#pragma once

#include <iostream>

#include <sys/wait.h>

#include "PipeFD.hpp"
#include "IoState.hpp"
#include "CGIConfig.hpp"

class CGIExecutor
{
private:
	int		_pid;

	PipeFD	_write_pipe[2];
	PipeFD	_read_pipe[2];

	void	_initPipes();
	void	_initFork();
	void	_initChild( CGIConfig & config );
	void	_restoreDefaultSignalMask();
	void	_initChildPipes();
	void	_dup2FD( int fd1, int fd2 );

	std::vector<char *>	_generateEnvp( CGIConfig & config );
	std::vector<char *>	_generateArgv( CGIConfig & config );

public:
	CGIExecutor() = delete;
	CGIExecutor( CGIConfig & config );
	~CGIExecutor() = default;

	int	getPID() const noexcept;

	int	releaseWriteFD() noexcept;
	int	releaseReadFD() noexcept;
};
