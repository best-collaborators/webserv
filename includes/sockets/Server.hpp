#pragma once

#include <iostream>
#include <map>
#include <unordered_set>

#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>

#include "Logger.hpp"
#include "Poller.hpp"
#include "Listener.hpp"
#include "Connection.hpp"
#include "CGIOperation.hpp"
#include "ChildExitInfo.hpp"
#include "CGIExitStatus.hpp"
#include "ChildSignalHandler.hpp"

extern volatile sig_atomic_t	g_running;

class Server
{
private:
	using connections_map = std::map<int, Connection>;
	using cgi_pipe_fds_set = std::unordered_set<int>;
	using fd_to_connection_map = std::unordered_map<int, Connection *>;
	using pid_to_connection_map = std::unordered_map<pid_t, Connection *>;

	std::chrono::seconds	_connection_timeout;

	Poller					_poller;
	Listener				_listener;
	ChildSignalHandler		_childHandler;

	connections_map			_connections;
	cgi_pipe_fds_set		_cgi_pipe_fds;

	fd_to_connection_map	_fd_to_connection;
	pid_to_connection_map	_pid_to_connection;

	void					_acceptConnection();
	void					_handleEvent( epoll_event const & event );
	void					_modifyEvent( int fd, uint32_t events ) noexcept;
	void					_closeConnection( int fd ) noexcept;
	void					_closeIdleConnections() noexcept;

	void					_registerConnectionCGI( Connection & connection, CGIOperation operation ) noexcept;
	void					_unregisterConnectionCGI( Connection & connection, CGIOperation operation ) noexcept;

	void					_handleConnectionEvent( IoEvent event, int fd );
	void					_handleCGIEvent( IoEvent event, Connection & connection );

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
