#include "Listener.hpp"
#include "Socket.hpp"

Listener::Listener( std::string const & port ) : _port(port)
{
	addrinfo *	address = nullptr;
	AddrInfoPtr	addresses_guard = getAddresses();

	for (address = addresses_guard.get(); address != nullptr; address = address->ai_next)
	{
		try
		{
			setupSocket(address);
		}
		catch( const std::exception & e )
		{
			std::cerr << e.what() << '\n';
			continue;
		}
		break;
	}

	if (address == nullptr)
		throw std::runtime_error("[listen] Bind failed for all addresses.");

	_socket.listen();

	std::cout << "[listen] Listening on port " << _port << "..." << std::endl;
}

Listener::AddrInfoPtr	Listener::getAddresses() const
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

void	Listener::setupSocket( addrinfo const * address )
{
	_socket.create(address);

	_socket.setNonBlocking();
	_socket.setAddressReuse();
	if (address->ai_family == AF_INET6)
		_socket.setDualStack();

	_socket.bind(address);
}

int	Listener::getFD() const
{
	return _socket.getFD();
}