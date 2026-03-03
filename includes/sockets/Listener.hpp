#pragma once

#include <netdb.h>

#include <memory>
#include <iostream>

#include "Socket.hpp"

class Listener
{
private:
	using	AddrInfoPtr = std::unique_ptr<addrinfo, void(*)(addrinfo *)>;

	Socket		_socket;
	std::string	_ip;
	std::string	_port;

	AddrInfoPtr			getAddresses() const;
	void				setupSocket( addrinfo const * address );

public:
	Listener() = delete;
	Listener( std::string const & ip, std::string const & port );

	Listener( Listener const & ) = delete;
	Listener & operator=( Listener const & ) = delete;

	Listener( Listener && other ) noexcept;
	Listener & operator=( Listener && other ) noexcept;

	~Listener() = default;

	Socket	accept();
	int		getFD() const;
};

