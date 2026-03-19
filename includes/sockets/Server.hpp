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
#include "ChildSignalHandler.hpp"
#include "ConfigurationFileParser.hpp"

extern volatile sig_atomic_t	g_running;

class Server
{
private:
	static constexpr int		CONNECTION_TIMEOUT = 1000;
	static constexpr int		CGI_TIMEOUT = 1000;
	struct ListenerEntry
	{
		Listener listener;
		ServerBlock const * server_block;

		ListenerEntry( Listener l, ServerBlock const * sb ) : listener(std::move(l)), server_block(sb)
		{}
	};

	using connections_map = std::map<int, Connection>;
	using cgi_pipe_fds_set = std::unordered_set<int>;
	using fd_to_connection_map = std::unordered_map<int, Connection *>;
	using pid_to_connection_map = std::unordered_map<pid_t, Connection *>;
	using server_blocks_map = ConfigurationFileParser::server_block_map;
	using listeners_map = std::unordered_map<int, ListenerEntry>;

	std::chrono::seconds	_connection_timeout;
	std::chrono::seconds	_cgi_timeout;

	server_blocks_map			_server_blocks;
	listeners_map			_listeners;

	Poller					_poller;
	ChildSignalHandler		_childHandler;

	connections_map			_connections;
	cgi_pipe_fds_set		_cgi_pipe_fds;

	fd_to_connection_map	_fd_to_connection;
	pid_to_connection_map	_pid_to_connection;

	void					_acceptConnection( int listener_fd );
	void					_handleEvent( epoll_event const & event );
	void					_modifyEvent( int fd, uint32_t events ) noexcept;
	void					_closeConnection( int fd ) noexcept;
	void					_handleTimeouts() noexcept;
	void					_handleConnectionTimeout( Connection & connection, std::vector<int> & to_close, std::chrono::_V2::steady_clock::time_point now ) noexcept;
	void					_handleCGITimeout( Connection & connection, std::vector<int> & to_close, std::chrono::_V2::steady_clock::time_point now ) noexcept;

	void					_registerConnectionCGI( Connection & connection, CGIOperation operation ) noexcept;
	void					_unregisterConnectionCGI( Connection & connection, CGIOperation operation ) noexcept;

	void					_handleConnectionEvent( IoEvent event, int fd );
	void					_handleCGIEvent( IoEvent event, Connection & connection );

	void					_handleFinishedChildren();
	Connection *			_getConnectionByFD( int fd );
	Connection *			_getConnectionByPID( pid_t fd );
public:
	Server() = delete;
	Server( ConfigurationFileParser::server_block_map server_blocks_map );

	Server( Server const & ) = delete;
	Server & operator=( Server const & ) = delete;

	Server( Server && ) noexcept = delete;
	Server & operator=( Server && ) noexcept = delete;

	~Server() = default;

	void	run();
};
