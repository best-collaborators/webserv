#pragma once

#include <iostream>
#include <string>
#include <system_error>

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

class Socket
{
private:
	static constexpr int _MAX_CONNECTIONS = 1024;

	int	_fd;

	void	_safeClose( int & fd ) noexcept;
	void	_checkStatus( int status, std::string const &message );

public:
	Socket();
	Socket( Socket const & ) = delete;
	Socket & operator=( Socket const & ) = delete;
	~Socket();

	void	create( addrinfo const * address );

	void	setNonBlocking();
	void	setAddressReuse();
	void	setDualStack();

	void	bind( addrinfo const * address );
	void	listen();

	int		getFD() const;
};
