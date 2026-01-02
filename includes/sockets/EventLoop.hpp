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

class EventLoop
{
private:
	static constexpr int	MAX_TRIGGERED_EVENTS = 10;
	static constexpr int	TIMEOUT = -1;
	static constexpr int	READ_BUFFER_SIZE = 4096;

	int	const				_listen_fd;
	int						_epoll_fd;
	epoll_event				triggered_events[MAX_TRIGGERED_EVENTS];

	void					createEpollInstance();
	void					registerListenSocket();

	int						monitorEvents( int & event_count );

	bool					isConnectionEvent( int fd ) const;
	bool					acceptNewConnection( int & connection_fd );
	bool					registerNewConnection( int & connection_fd );
	bool					handleClientEvent( epoll_event & event );

public:
	EventLoop( int listen_fd );
	~EventLoop();

	void					init();
	void					monitor();
};
