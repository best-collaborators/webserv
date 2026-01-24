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

Server::Server(std::string const &port) : _poller(), _listener(port)
{
	if (_poller.add(_listener.getFD(), EPOLLIN) == false)
	{
		throw std::system_error(errno, std::generic_category(), "[epoll] EPOLL_CTL_ADD listen_fd failed");
	}

	int childHandleFD = _childHandler.getFD();

	if (!_poller.add(childHandleFD, EPOLLIN))
		throw std::system_error(errno, std::generic_category(), "[epoll] EPOLL_CTL_ADD childHandleFD failed");

	Log::info("Added listen_fd " + std::to_string(_listener.getFD()) + " (EPOLLIN)", "epoll");
}

void Server::run()
{
	Log::info("Waiting for connection...", "accept");

	while (g_running)
	
	{
		// if (_connections.empty())
		// {
		// 	_shutdown_tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_CLOEXEC);
		// 	itimerspec ts {};
		// 	ts.it_value.tv_sec = 10;

		// 	timerfd_settime(_shutdown_tfd, 0, &ts, nullptr);

		// 	_poller.add(_shutdown_tfd, EPOLLIN);
		// 	std::cout << "Start shutdown timer" << std::endl;
		// }

		if (!_connections.empty())
		{
			for (auto it = _connections.begin(); it != _connections.end();)
			{
				std::cout << "connections size: " << _connections.size() << std::endl;
				if (it->first >= 0)
				{
					std::chrono::seconds	s{5};
					auto	now = std::chrono::steady_clock::now();
					auto	duration = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.getLastActivity());
		
					if (duration >= s)
					{
						std::cout << "CLOSE! fd: " << it->first << ", duration: " << duration.count() << std::endl;
						closeConnection(it->first);
						it = _connections.erase(it);
						continue;
					}
					else
					{
						std::cout << "fd: " << it->first << ", duration: " << duration.count() << std::endl;
					}
				}
				std::cout << "end" << std::endl;
				++it;
			}
		}
		std::cout << "exit loop" << std::endl;

		int event_count = _poller.wait();
		std::cout << "Wait" << std::endl;

		for (int i = 0; i < event_count; ++i)
		{
			epoll_event const &event = _poller.getEvent(i);
			int fd = event.data.fd;

			if (fd == _listener.getFD())
				_acceptConnection();
			else if (fd == _childHandler.getFD())
				_handleFinishedChildren();
			else
				_handleEvent(event);
		}
	}
}

void Server::_acceptConnection()
{
	Socket client = _listener.accept();
	int client_fd = client.getFD();

	if (client_fd == -1)
		return;

	Log::info("New connection fd " + std::to_string(client_fd), "accept");

	if (!_poller.add(client_fd, EPOLLIN))
		return;

	_connections.emplace(client_fd, std::move(client));
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
	{
		Log::warning("getCGIpipe returned already closed fd", "CGI");
		return;
	}

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
	auto children = _childHandler.handleFinishedChildren();

	for (auto const & child : children)
	{
		Connection * connection = _getConnectionByPID(child.pid);

		if (!connection)
			continue;

		EventAction action = connection->onChildProcessExited(child);

		_pid_to_connection.erase(child.pid);

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
