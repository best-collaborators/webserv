#include "Socket.hpp"

Socket::Socket() : _fd(-1)
{}

Socket::~Socket()
{
	_safeClose(_fd);
}

void	Socket::create( addrinfo const * address )
{
	_safeClose(_fd);

	_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	_checkStatus(_fd, "[socket] socket() failed");

	std::cout << "[socket] Socket created." << std::endl;
}

void	Socket::setNonBlocking()
{
	int	flags = fcntl(_fd, F_GETFL, 0);

	_checkStatus(flags, "[socket] fcntl(F_GETFL) failed");

	int	status = fcntl(_fd, F_SETFL, flags | O_NONBLOCK);

	_checkStatus(status, "[socket] fcntl(O_NONBLOCK) failed");
}

void	Socket::setAddressReuse()
{
	int	reuse_address = 1;
	int status = setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address));

	_checkStatus(status, "[socket] setsockopt(SO_REUSEADDR) failed");
}

void	Socket::setDualStack()
{
	int	ipv6_only = 0;
	int status = setsockopt(_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));

	_checkStatus(status, "[socket] setsockopt(IPV6_V6ONLY) failed");
}

void	Socket::bind( addrinfo const * address )
{
	int	status = ::bind(_fd, address->ai_addr, address->ai_addrlen);

	_checkStatus(status, "[socket] bind() failed");

	std::cout << "[socket] Socket bound." << std::endl;
}

void	Socket::listen()
{
	int	status = ::listen(_fd, _MAX_CONNECTIONS);

	_checkStatus(status, "[socket] listen() failed");
}

void	Socket::_checkStatus( int status, std::string const &message )
{
	if (status == -1)
	{
		throw std::system_error(errno, std::generic_category(), message);
	}
}

int	Socket::getFD() const
{
	return _fd;
}

void	Socket::_safeClose( int & fd ) noexcept
{
	if (fd != -1)
	{
		while (close(fd) == -1 && errno == EINTR) {}

		fd = -1;
	}
}
