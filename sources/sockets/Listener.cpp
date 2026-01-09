#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/SocketUtils.hpp"

Listener::Listener( std::string const & port ) : _port(port), _listen_fd(-1)
{
	addrinfo *	address = nullptr;
	AddrInfoPtr	addresses_guard = getAddresses();

	for (address = addresses_guard.get(); address != nullptr; address = address->ai_next)
	{
		try
		{
			createSocket(address);
			configureSocket(address);
			bindSocket(address);
		}
		catch( const std::exception & e )
		{
			std::cerr << e.what() << '\n';
			SocketUtils::safeCloseFD(_listen_fd);
			continue;
		}
		break;
	}

	if (address == nullptr)
	{
		SocketUtils::safeCloseFD(_listen_fd);
		throw std::runtime_error("[listen] Bind failed for all addresses.");
	}

	listenSocket();
}

Listener::~Listener()
{
	SocketUtils::safeCloseFD(_listen_fd);
}

int	Listener::getFD() const
{
	return this->_listen_fd;
}

Listener::AddrInfoPtr Listener::getAddresses() const
{
	addrinfo	hints {};
	addrinfo *	addresses = nullptr;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int	status = getaddrinfo(nullptr, _port.c_str(), &hints, &addresses);

	if (status != 0)
	{
		throw std::runtime_error("[gai] Failed to get socket addresses: " + std::string(gai_strerror(status)));
	}

	return AddrInfoPtr(addresses, freeaddrinfo);
}

void	Listener::createSocket( addrinfo const * address )
{
	_listen_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	SocketUtils::checkStatus(_listen_fd, "[listen] socket() failed");

	std::cout << "[listen] Socket created." << std::endl;
}

void	Listener::configureSocket( addrinfo const * address )
{
	setNonBlocking();
	setAddressReuse();
	if (address->ai_family == AF_INET6)
		setDualStack();
}

void	Listener::bindSocket( addrinfo const * address )
{
	int	status = bind(_listen_fd, address->ai_addr, address->ai_addrlen);

	SocketUtils::checkStatus(status, "[listen] bind() failed");

	std::cout << "[listen] Socket bound." << std::endl;
}

void	Listener::listenSocket()
{
	int	status = listen(_listen_fd, Listener::MAX_CONNECTIONS);

	SocketUtils::checkStatus(status, "[listen] listen() failed");

	std::cout << "[listen] Listening on port " << _port << "..." << std::endl;
}

void	Listener::setNonBlocking()
{
	int flags = fcntl(_listen_fd, F_GETFL, 0);
	SocketUtils::checkStatus(flags, "[listen] fcntl(F_GETFL) failed");

	int status = fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK);

	SocketUtils::checkStatus(status, "[listen] fcntl(O_NONBLOCK) failed");
}

void	Listener::setAddressReuse()
{
	int	reuse_address = 1;
	int status = setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address));

	SocketUtils::checkStatus(status, "[listen] setsockopt(SO_REUSEADDR) failed");
}

void	Listener::setDualStack()
{
	int	ipv6_only = 0;
	int status = setsockopt(_listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));

	SocketUtils::checkStatus(status, "[listen] setsockopt(IPV6_V6ONLY) failed");
}
