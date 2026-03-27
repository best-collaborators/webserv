#include "Listener.hpp"
#include "Socket.hpp"

Listener::Listener( std::string const & ip, std::string const & port ) : _ip(ip), _port(port)
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
		throw std::runtime_error("[listen] Bind failed for " + _ip + ":" + _port);

	_socket.listen();

	Log::info("Listening on " + _ip + ":" + _port + "...", "listen");
}

Listener::Listener( Listener && other ) noexcept
	: _socket(std::move(other._socket)), _ip(std::move(other._ip)), _port(std::move(other._port))
{}

Listener &Listener::operator=( Listener && other ) noexcept
{
	if (this != &other)
	{
		_socket = std::move(other._socket);
		_ip = std::move(other._ip);
		_port = std::move(other._port);
	}
	return *this;
}

Listener::AddrInfoPtr	Listener::getAddresses() const
{
	addrinfo	hints {};
	addrinfo *	addresses = nullptr;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int	status = getaddrinfo(_ip.empty() ? nullptr : _ip.c_str(), _port.c_str(), &hints, &addresses);

	if (status != 0)
	{
		throw std::runtime_error("[gai] Failed to get socket addresses: " + std::string(gai_strerror(status)));
	}

	return AddrInfoPtr(addresses, freeaddrinfo);
}

void	Listener::setupSocket( addrinfo const * address )
{
	_socket.create(address);

	_socket.setAddressReuse();
	if (address->ai_family == AF_INET6)
		_socket.setDualStack();

	try
	{
		_socket.bind(address, _ip, _port);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

Socket	Listener::accept()
{
	return _socket.accept();
}

int	Listener::getFD() const
{
	return _socket.getFD();
}