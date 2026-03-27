#pragma once

#include <iostream>

#include <sys/wait.h>

class PipeFD
{
private:
	int	_fd;

public:
	PipeFD();
	PipeFD( int fd ) noexcept;

	PipeFD( PipeFD const & ) = delete;
	PipeFD & operator=( PipeFD const & ) = delete;

	PipeFD( PipeFD && other ) noexcept;
	PipeFD & operator=( PipeFD && other ) noexcept;

	~PipeFD();

	int		release() noexcept;
	void	reset();
	int		get() const noexcept;
};