#include "../includes/sockets/SocketUtils.hpp"

void	SocketUtils::checkStatus( int status, std::string const &message )
{
	if (status == -1)
	{
		throw std::system_error(errno, std::generic_category(), message);
	}
}

void	SocketUtils::safeCloseFD( int & fd ) noexcept
{
	if (fd != -1)
	{
		while (close(fd) == -1 && errno == EINTR) {}

		fd = -1;
	}
}
