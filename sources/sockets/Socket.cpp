#include "Socket.hpp"

Socket::Socket() : _fd(-1)
{}

Socket::Socket( int fd ) : _fd(fd)
{
	_setNonBlocking();
}

Socket::Socket( Socket && other ) noexcept : _fd(other._fd) 
{
	other._fd = -1;
}

Socket &	Socket::operator=( Socket && other ) noexcept
{
	if (this != &other)
	{
		_safeClose();
		_fd = other._fd;
		other._fd = -1;
	}
	return *this;
}

Socket::~Socket()
{
	_safeClose();
}

void	Socket::create( addrinfo const * address )
{
	_safeClose();

	_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	_checkStatus(_fd, "[socket] socket() failed");

	_setNonBlocking();

	Log::info("Socket created", "socket");
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

void	Socket::bind( addrinfo const * address, std::string const & ip, std::string const & port )
{
	int	status = ::bind(_fd, address->ai_addr, address->ai_addrlen);

	_checkStatus(status, "[socket] binding address " + ip + ":" + port + " failed");

	Log::info("Socket bound", "socket");
}

void	Socket::listen()
{
	int	status = ::listen(_fd, _MAX_CONNECTIONS);

	_checkStatus(status, "[socket] listen() failed");
}

Socket	Socket::accept()
{
	sockaddr_storage	connection_address {};
	socklen_t			connection_address_size {};

	int	fd = ::accept(_fd, reinterpret_cast<sockaddr *>(&connection_address), &connection_address_size);
	Log::info("accept() returned", "accept");

	if (fd == - 1)
	{
		Log::error("[accept] Failed (" + std::to_string(errno) + "): " + strerror(errno), "accept");
		return Socket();
	}

	return Socket(fd);
}

int	Socket::getFD() const
{
	return _fd;
}

bool	Socket::isHealthy() const noexcept
{
	int			error = 0;
	socklen_t	len = sizeof(error);

	if (!getsockopt(_fd, SOL_SOCKET, SO_ERROR, &error, &len))
	{
		if (error == 0)
		{
			return true;
		}
	}

	return false;
}

void	Socket::_setNonBlocking()
{
	int	flags = fcntl(_fd, F_GETFL, 0);

	_checkStatus(flags, "[socket] fcntl(F_GETFL) failed");

	int	status = fcntl(_fd, F_SETFL, flags | O_NONBLOCK);

	_checkStatus(status, "[socket] fcntl(O_NONBLOCK) failed");
}

void	Socket::_safeClose() noexcept
{
	if (_fd != -1)
	{
		while (close(_fd) == -1 && errno == EINTR) {}

		_fd = -1;
	}
}

void	Socket::_checkStatus( int status, std::string const &message ) const
{
	if (status == -1)
	{
		throw std::system_error(errno, std::generic_category(), message);
	}
}
