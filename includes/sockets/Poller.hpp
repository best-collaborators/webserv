#pragma once

#include <vector>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

class Poller
{
private:
	static constexpr int		MAX_EVENTS = 1024;
	static constexpr int		TIMEOUT = -1; //! Handle timeout

	int							_epoll_fd;
	std::vector<epoll_event>	_events;

	bool	control( int operation, int fd, uint32_t events );

public:
	Poller();
	Poller( Poller const & ) = delete;
	Poller & operator=( Poller const & ) = delete;
	~Poller();

	int		wait();

	bool	add( int fd, uint32_t events );
	bool	mod( int fd, uint32_t events );
	bool	del( int fd );

	epoll_event const &	getEvent( int index ) const;
};
