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

	std::cout << "[epoll] Added listen_fd " << _listener.getFD() << " (EPOLLIN)." << std::endl;
}

void Server::run()
{
	std::cout << "\n[accept] Waiting for connection..." << std::endl;

	while (g_running)
	{
		int event_count = _poller.wait();

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

	std::cout << "[accept] New connection fd " << client_fd << std::endl;

	if (!_poller.add(client_fd, EPOLLIN))
		return;

	_connections.emplace(client_fd, std::move(client));
	Connection &connection = _connections.at(client_fd);
	_fd_to_connection[client_fd] = &connection;

	std::cout << "[epoll] Register new connection " << client_fd << " (EPOLLIN)." << std::endl;
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
			std::cout << "Connection - IoEvent::Error" << std::endl;
			std::cerr << "[io] EPOLLERR on connection fd " << fd << std::endl;
			_closeConnection(fd);
			break;

		case IoEvent::Closed:
			std::cout << "Connection - IoEvent::Closed" << std::endl;
			std::cout << "[io] EPOLLHUP on connection fd " << fd << std::endl;
			_closeConnection(fd);
			break;

		case IoEvent::Received:
			std::cout << "IoEvent::Received" << std::endl;
			_modifyEvent(fd, EPOLLIN | EPOLLOUT);
			break;

		case IoEvent::Sent:
			std::cout << "IoEvent::Sent" << std::endl;
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
			std::cout << "CGI - IoEvent::Error" << std::endl;
			_unregisterConnectionCGI(connection, CGIOperation::WRITE);
			_unregisterConnectionCGI(connection, CGIOperation::READ);
			break;

		case IoEvent::Sent:
			std::cout << "CGI - IoEvent::Sent" << std::endl;
			_unregisterConnectionCGI(connection, CGIOperation::WRITE);
			_registerConnectionCGI(connection, CGIOperation::READ);
			break;

		case IoEvent::Init:
			std::cout << "CGI - IoEvent::Init" << std::endl;
			_registerConnectionCGI(connection, CGIOperation::WRITE);
			_pid_to_connection[connection.getCGIPID()] = &connection;
			break;

		case IoEvent::Done:
			std::cout << "CGI - IoEvent::Done" << std::endl;
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
		std::cout << "_modifyEvent event wasn't modified" << std::endl;
		_closeConnection(fd);
	}
	else
		std::cout << "[epoll] Updated fd " << fd << " to " << eventsToString(events) << "." << std::endl;
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
	// std::cout << "[connection] Closed and removed fd " << fd << std::endl;

}

void Server::_registerConnectionCGI(Connection &connection, CGIOperation operation) noexcept
{
	int fd = connection.getCGIPipe(operation);

	if (fd == -1)
	{
		std::cerr << "[CGI] (Server::_registerConnectionCGI) no valid fd " << std::endl;
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
		std::cerr << "[CGI] Failed to register fd " << fd << " with epoll" << std::endl;
}

void Server::_unregisterConnectionCGI(Connection &connection, CGIOperation op) noexcept
{
	int fd = connection.getCGIPipe(op);

	if (fd == -1)
	{
		// Logger::displayLog(Logger::e_log_level::ERROR, "no valid fd", "[CGI] (Server::closeCGI)");
		std::cerr << "[CGI] (Server::closeCGI) no valid fd " << std::endl;
		return;
	}

	if (_cgi_pipe_fds.erase(fd))
	{
		_poller.del(fd);
		_fd_to_connection.erase(fd);
		std::cout << "[CGI] fd " << fd << " removed from EPOLL" << std::endl;
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
