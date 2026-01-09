#pragma once

#include <iostream>

//send, sockaddr_storage, recv, accept, sockaddr
#include <sys/socket.h>

// strlen, strerror
#include <cstring>

// close
#include <unistd.h>

// epoll_create1, epoll_ctl, epoll_wait, epoll_event, EPOLLIN, EPOLL_CTL_ADD
#include <sys/epoll.h>

// fcntl, F_SETFL, O_NONBLOCK
#include <fcntl.h>

#include <map>

#include "Connection.hpp"
#include "Poller.hpp"

class EventLoop
{
private:
	Poller						_poller;
	int	const					_listen_fd;
	std::map<int, Connection>	connections;

	bool					acceptNewConnection( int & connection_fd );
	void					registerNewConnection( int & fd ) noexcept;
	void					handleClientEvent( epoll_event const & event ) noexcept;

	void					registerEventToReadWrite( int fd ) noexcept;
	void					registerEventToReadOnly( int fd ) noexcept;

	void					closeConnection( int fd ) noexcept;

public:
	EventLoop( int listen_fd );
	~EventLoop();

	void	run();
};
