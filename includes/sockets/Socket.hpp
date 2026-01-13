#pragma once

#include <iostream>
#include <string>
#include <system_error>
#include <cstring>

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

class Socket
{
private:
	static constexpr int _MAX_CONNECTIONS = 1024;

	int	_fd;

	void	_setNonBlocking();
	void	_safeClose() noexcept;
	void	_checkStatus( int status, std::string const &message ) const;

public:
	Socket();
	Socket( int fd );

	Socket( Socket const & ) = delete;
	Socket & operator=( Socket const & ) = delete;

	Socket( Socket && other ) noexcept;
	Socket & operator=( Socket && other ) noexcept;

	~Socket();

	void	create( addrinfo const * address );

	void	setAddressReuse();
	void	setDualStack();

	void	bind( addrinfo const * address );
	void	listen();
	Socket	accept();

	int		getFD() const;

	bool	isHealthy() const noexcept;
};
