#pragma once

#include <iostream>

// epoll_create1, epoll_ctl, epoll_wait, epoll_event, EPOLLIN, EPOLL_CTL_ADD
#include <sys/epoll.h>

// fcntl, F_SETFL, O_NONBLOCK
#include <fcntl.h>
#include <signal.h>

#include <map>

#include "Listener.hpp"
#include "Connection.hpp"
#include "Poller.hpp"

extern volatile sig_atomic_t	g_running;

class Server
{
private:
	Listener					_listener;
	Poller						_poller;
	std::map<int, Connection>	_connections;

	void					acceptConnection();
	void					handleEvent( epoll_event const & event ) noexcept;
	void					modifyEvent( int fd, uint32_t events ) noexcept;
	void					closeConnection( int fd ) noexcept;

public:
	Server() = delete;
	Server( std::string const & port );

	Server( Server const & ) = delete;
	Server & operator=( Server const & ) = delete;

	Server( Server && ) noexcept = delete;
	Server & operator=( Server && ) noexcept = delete;

	~Server() = default;

	void	run();
};
