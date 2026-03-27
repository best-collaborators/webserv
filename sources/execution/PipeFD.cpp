#include "PipeFD.hpp"

PipeFD::PipeFD() : _fd(-1)
{}

PipeFD::PipeFD( int fd ) noexcept : _fd(fd)
{}

PipeFD::PipeFD( PipeFD && other ) noexcept : _fd(other._fd)
{
	other._fd = -1;
}

PipeFD &PipeFD::operator=(PipeFD &&other) noexcept
{
	if (this != &other)
	{
		if (_fd != -1)
			close(_fd);

		_fd = other._fd;
		other._fd = -1;
	}

	return *this;
}

PipeFD::~PipeFD()
{
	if (_fd != -1)
		close(_fd);
}

int PipeFD::release() noexcept
{
	int fd = _fd;
	_fd = -1;

	return fd;
}

void PipeFD::reset()
{
	if (_fd != -1)
		close(_fd);

	_fd = -1;
}

int PipeFD::get() const noexcept
{
	return _fd;
}