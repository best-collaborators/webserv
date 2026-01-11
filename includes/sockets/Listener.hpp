#pragma once

#include <memory>

#include "Socket.hpp"

class Listener
{
private:
	using	AddrInfoPtr = std::unique_ptr<addrinfo, void(*)(addrinfo *)>;

	Socket				_socket;
	std::string const	_port;

	AddrInfoPtr			getAddresses() const;
	void				setupSocket( addrinfo const * address );

public:
	Listener( std::string const & port );
	~Listener() = default;

	int					getFD() const;
};

