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
#include "CGIOperation.hpp"
#include "ChildExitInfo.hpp"
#include "CGIExitStatus.hpp"
#include "ChildSignalHandler.hpp"

extern volatile sig_atomic_t	g_running;

class Server
{
private:
	Listener					_listener;
	Poller						_poller;
	std::map<int, Connection>	_connections;
	std::unordered_map<int, Connection *>	_fd_to_connection;
	std::unordered_map<pid_t, Connection *>	_pid_to_connection;
	ChildSignalHandler						_childHandler;

	void					acceptConnection();
	void					_handleEvent( epoll_event const & event );
	void					modifyEvent( int fd, uint32_t events ) noexcept;
	void					closeConnection( int fd ) noexcept;

	void					_registerConnectionCGI( Connection & connection, CGIOperation operation ) noexcept;
	void					_unregisterConnectionCGI( Connection & connection, CGIOperation operation ) noexcept;

	void					_handleError( Connection & connection, int fd, bool isActiveCGI );
	void					_handleClose( int fd, bool isActiveCGI );
	void					_handleReceived( int fd );
	void					_handleSent( Connection & connection, int fd, bool isActiveCGI );
	void					_handleCGIInit( Connection & connection );
	void					_handleCGIDone( Connection & connection );

	void					_handleFinishedChildren();
	Connection *			_getConnectionByFD( int fd );
	Connection *			_getConnectionByPID( pid_t fd );
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
