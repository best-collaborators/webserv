#pragma once

#include <iostream>

// getaddrinfo, freeaddrinfo, gai_strerror, addrinfo, AI_PASSIVE, IPPROTO_IPV6, IPV6_V6ONLY
#include <netdb.h>

// socket, setsockopt, bind, listen, AF_UNSPEC, SOCK_STREAM, AF_INET, SOL_SOCKET, SO_REUSEADDR
#include <sys/socket.h>

// fcntl, F_SETFL, O_NONBLOCK
#include <fcntl.h>

// strerror
#include <cstring>

// close
#include <unistd.h>

#include <memory>

class Listener
{
private:
	static constexpr int MAX_CONNECTIONS = 10;

	using	AddrInfoPtr = std::unique_ptr<addrinfo, void(*)(addrinfo *)>;

	std::string const	_port;
	int					_listen_fd;

	AddrInfoPtr			getAddresses() const;

	void				createSocket( addrinfo const * address );
	void				configureSocket( addrinfo const * address );
	void				bindSocket( addrinfo const * address );
	void				listenSocket();

	void				setNonBlocking();
	void				setAddressReuse();
	void				setDualStack();

public:
	Listener( std::string const & port );
	~Listener();

	void				init();
	int					getListenFd() const;
};

