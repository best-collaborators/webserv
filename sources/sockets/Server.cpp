#include "Server.hpp"

volatile sig_atomic_t g_running = true;

static std::string eventsToString( uint32_t events )
{
	if (events == (EPOLLIN | EPOLLOUT))
		return "EPOLLIN | EPOLLOUT";
	if (events == EPOLLIN)
		return "EPOLLIN";
	if (events == EPOLLOUT)
		return "EPOLLOUT";
	return "UNKNOWN";
}

Server::Server( ConfigurationFileParser::server_block_map server_blocks )
	: _connection_timeout(CONNECTION_TIMEOUT), _cgi_timeout(CGI_TIMEOUT), _server_blocks(std::move(server_blocks)), _poller()
{
	for (auto const & [listen, block] : _server_blocks)
	{
		Listener listener(listen.ip_address, std::to_string(listen.port));

		int listener_fd = listener.getFD();

		_listeners.try_emplace(listener_fd, ListenerEntry(std::move(listener), &block));

		if (_poller.add(listener_fd, EPOLLIN) == false)
			throw std::system_error(errno, std::generic_category(), "[epoll] EPOLL_CTL_ADD listen_fd failed");

		Log::info("Added listen_fd " + std::to_string(listener_fd) + " (EPOLLIN)", "epoll");
	}

	int childHandleFD = _childHandler.getFD();

	if (!_poller.add(childHandleFD, EPOLLIN))
		throw std::system_error(errno, std::generic_category(), "[epoll] EPOLL_CTL_ADD childHandleFD failed");
}

void Server::run()
{
	Log::info("Waiting for connection...", "accept");

	while (g_running)
	{
		_handleTimeouts();

		int event_count = _poller.wait();

		for (int i = 0; i < event_count; ++i)
		{
			epoll_event const &event = _poller.getEvent(i);
			int fd = event.data.fd;

			if (_listeners.count(fd))
				_acceptConnection(fd);
			else if (fd == _childHandler.getFD())
				_handleFinishedChildren();
			else
				_handleEvent(event);
		}
	}
}

void Server::_acceptConnection( int listener_fd )
{
	auto & [listener, server_block] = _listeners.at(listener_fd);

	Socket client = listener.accept();
	int client_fd = client.getFD();

	if (client_fd == -1)
		return;

	Log::info("New connection fd " + std::to_string(client_fd), "accept");

	if (!_poller.add(client_fd, EPOLLIN))
		return;

	_connections.try_emplace(client_fd, server_block, std::move(client));
	Connection &connection = _connections.at(client_fd);
	_fd_to_connection[client_fd] = &connection;

	Log::info("Register new connection " + std::to_string(client_fd) + " (EPOLLIN)", "epoll");
}

void Server::_handleEvent(epoll_event const &event)
{
	int fd = event.data.fd;

	Connection * connection = _getConnectionByFD(fd);

	if (!connection)
		return;

	bool	isCGIPipe = _cgi_pipe_fds.count(fd);

	IoResult result = isCGIPipe
		? connection->processCGIEvents(event.events) 
		: connection->processConnectionEvents(event.events);

	if (result.event == IoEvent::Pending)
		return;

	if (result.source == IoSource::Connection)
		_handleConnectionEvent(result.event, fd);
	else
		_handleCGIEvent(result.event, *connection);
}

void Server::_handleConnectionEvent( IoEvent event, int fd )
{
	switch (event)
	{
		case IoEvent::Error:
			Log::warning("IoEvent::Error", "Connection");
			Log::warning("EPOLLERR on connection fd " + std::to_string(fd), "Connection");
			_closeConnection(fd);
			break;

		case IoEvent::Closed:
			Log::warning("IoEvent::Closed", "Connection");
			Log::warning("EPOLLHUP on connection fd " + std::to_string(fd), "Connection");
			_closeConnection(fd);
			break;

		case IoEvent::Received:
			Log::debug("IoEvent::Received", "Connection");
			_modifyEvent(fd, EPOLLIN | EPOLLOUT);
			break;

		case IoEvent::Sent:
			Log::debug("IoEvent::Sent", "Connection");
			_modifyEvent(fd, EPOLLIN);
			break;

		default:
			break;
	}
}

void Server::_handleCGIEvent( IoEvent event, Connection & connection )
{
	switch (event)
	{
		case IoEvent::Error:
			Log::warning("IoEvent::Error", "CGI");
			_unregisterConnectionCGI(connection, CGIOperation::WRITE);
			_unregisterConnectionCGI(connection, CGIOperation::READ);
			connection.abortCGIWithError();
			_modifyEvent(connection.getFD(), EPOLLIN | EPOLLOUT);
			break;

		case IoEvent::Sent:
			Log::debug("IoEvent::Sent", "CGI");
			_unregisterConnectionCGI(connection, CGIOperation::WRITE);
			_registerConnectionCGI(connection, CGIOperation::READ);
			break;

		case IoEvent::Init:
			Log::debug("IoEvent::Init", "CGI");
			_registerConnectionCGI(connection, CGIOperation::WRITE);
			_pid_to_connection[connection.getCGIPID()] = &connection;
			break;

		case IoEvent::Done:
			Log::debug("IoEvent::Done", "CGI");
			_unregisterConnectionCGI(connection, CGIOperation::READ);
			{
				EventAction	action = connection.onCGIOutputReady();
				if (action == EventAction::EnableOutput)
					_modifyEvent(connection.getFD(), EPOLLIN | EPOLLOUT);
			}
			break;

		default:
			break;
	}
}

void Server::_modifyEvent(int fd, uint32_t events) noexcept
{
	if (_poller.mod(fd, events) == false)
	{
		Log::warning("Event wasn't modified", "Server");
		_closeConnection(fd);
	}
	else
		Log::debug("Updated fd " + std::to_string(fd) + " to " + eventsToString(events), "epoll");
}

void Server::_closeConnection(int fd) noexcept
{
	Connection * connection = _getConnectionByFD(fd);
	if (connection) {
		_unregisterConnectionCGI(*connection, CGIOperation::READ);
		_unregisterConnectionCGI(*connection, CGIOperation::WRITE);

		int pid = connection->getCGIPID();
		if (pid > 0)
			_pid_to_connection.erase(pid);
	}
	_poller.del(fd);
	_connections.erase(fd);
	_fd_to_connection.erase(fd);
	Log::debug("Closed and removed fd " + std::to_string(fd), "Connection");
}

void Server::_registerConnectionCGI(Connection &connection, CGIOperation operation) noexcept
{
	int fd = connection.getCGIPipe(operation);

	if (fd == -1)
	{
		Log::warning("getCGIpipe returned already closed fd", "CGI");
		return;
	}

	uint32_t event;

	if (operation == CGIOperation::WRITE)
		event = EPOLLOUT;
	else
		event = EPOLLIN;

	if (_poller.add(fd, event))
	{
		_fd_to_connection[fd] = &connection;
		_cgi_pipe_fds.insert(fd);
	}
	else
		Log::warning("Failed to register fd " + std::to_string(fd) + " with epoll", "CGI");
}

void Server::_unregisterConnectionCGI(Connection &connection, CGIOperation op) noexcept
{
	int fd = connection.getCGIPipe(op);

	if (fd == -1)
		return;

	if (_cgi_pipe_fds.erase(fd))
	{
		_poller.del(fd);
		_fd_to_connection.erase(fd);
		Log::debug("fd " + std::to_string(fd) + " removed from EPOLL", "CGI");
	}
	connection.closeCGIPipe(op);
}

void Server::_handleFinishedChildren()
{
	auto finished_pids = _childHandler.handleFinishedChildren();

	for (auto const & pid : finished_pids)
	{
		Connection * connection = _getConnectionByPID(pid);

		if (!connection)
			continue;

		EventAction action = connection->onChildProcessExited();

		_pid_to_connection.erase(pid);

		if (action == EventAction::EnableOutput)
			_modifyEvent(connection->getFD(), EPOLLIN | EPOLLOUT);
	}
}

Connection * Server::_getConnectionByFD(int fd)
{
	auto it = _fd_to_connection.find(fd);

	if (it == _fd_to_connection.end())
		return nullptr;

	return it->second;
}

Connection * Server::_getConnectionByPID(pid_t pid)
{
	auto it = _pid_to_connection.find(pid);

	if (it == _pid_to_connection.end())
		return nullptr;

	return it->second;
}

void Server::_handleTimeouts() noexcept
{
	if (_connections.empty())
		return;

	Log::debug("connections size: " + std::to_string(_connections.size()), "timeout");

	auto	now = std::chrono::steady_clock::now();
	std::vector<int>	to_close;

	for (auto & [_, connection] : _connections)
	{
		if (connection.getCGIStartTime().has_value())
		{
			_handleCGITimeout(connection, to_close, now);
			continue;
		}

		_handleConnectionTimeout(connection, to_close, now);
	}

	for (int fd : to_close)
		_closeConnection(fd);
}

void Server::_handleConnectionTimeout( Connection & connection, std::vector<int> & to_close, std::chrono::_V2::steady_clock::time_point now ) noexcept
{
	int fd = connection.getFD();
	auto	duration = std::chrono::duration_cast<std::chrono::seconds>(now - connection.getLastActivity());

	if (duration >= _connection_timeout)
	{
		Log::debug("Timeout! fd: " + std::to_string(fd) + ", duration: " + std::to_string(duration.count()), "timeout");
		to_close.push_back(fd);
	}
	else
	{
		Log::debug("fd: " + std::to_string(fd) + ", duration: " + std::to_string(duration.count()), "timeout");
	}
}

void Server::_handleCGITimeout( Connection & connection, std::vector<int> & to_close, std::chrono::_V2::steady_clock::time_point now ) noexcept
{
	int fd = connection.getFD();

	auto cgi_duration = std::chrono::duration_cast<std::chrono::seconds>(now - connection.getCGIStartTime().value());

	if (cgi_duration >= _cgi_timeout)
	{
		Log::warning("CGI timeout! fd: " + std::to_string(fd) + ", duration: " + std::to_string(cgi_duration.count()) + "s", "timeout");

		pid_t pid = connection.getCGIPID();

		if (pid > 0)
			kill(pid, SIGKILL);

		_unregisterConnectionCGI(connection, CGIOperation::WRITE);
		_unregisterConnectionCGI(connection, CGIOperation::READ);

		if (pid > 0)
			_pid_to_connection.erase(pid);

		if (connection.headersSentToClient())
		{
			Log::warning("CGI timeout after headers sent, closing connection fd: " + std::to_string(fd), "timeout");
			to_close.push_back(fd);
		}
		else
		{
			connection.abortCGI();
			_modifyEvent(fd, EPOLLIN | EPOLLOUT);
		}
	}
	else
	{
		Log::debug("CGI active on fd: " + std::to_string(fd) + ", cgi duration: " + std::to_string(cgi_duration.count()) + "s", "timeout");
		connection.resetLastActivity();
	}
}
