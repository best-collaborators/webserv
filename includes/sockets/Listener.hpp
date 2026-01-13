#pragma once

#include <netdb.h>

#include <memory>
#include <iostream>

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
	Listener() = delete;
	Listener( std::string const & port );

	Listener( Listener const & ) = delete;
	Listener & operator=( Listener const & ) = delete;

	Listener( Listener && ) noexcept = delete;
	Listener & operator=( Listener && ) noexcept = delete;

	~Listener() = default;

	Socket	accept();
	int		getFD() const;
};

