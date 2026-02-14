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

Server::Server(std::string const &port) : _listener(port), _poller()
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
				acceptConnection();
			else if (fd == _childHandler.getFD())
				_handleFinishedChildren();
			else
				_handleEvent(event);
		}
	}
}

void Server::acceptConnection()
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

	IoState state = connection->processEvents(event.events);

	bool isActiveCGI = connection->hasActiveCGI();

	switch (state)
	{
	case IoState::Error:
		_handleError(*connection, fd, isActiveCGI);
		break;
	case IoState::Closed:
		_handleClose(fd, isActiveCGI);
		break;
	case IoState::Received:
		_handleReceived(fd);
		break;
	case IoState::Sent:
		_handleSent(*connection, fd, isActiveCGI);
		break;
	case IoState::CGIInit:
		_handleCGIInit(*connection);
		break;
	case IoState::CGIDone:
		_handleCGIDone(*connection);
		break;
	default:
		break;
	}
}

void Server::modifyEvent(int fd, uint32_t events) noexcept
{
	if (_poller.mod(fd, events) == false)
	{
		std::cout << "modifyEvent event wasn't modified" << std::endl;
		closeConnection(fd);
	}
	else
		std::cout << "[epoll] Updated fd " << fd << " to " << eventsToString(events) << "." << std::endl;
}

void Server::closeConnection(int fd) noexcept
{
	if (_poller.del(fd) == true)
	{
		_connections.erase(fd);
		_fd_to_connection.erase(fd);

		std::cout << "[connection] Closed and removed fd " << fd << std::endl;
	}
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
	}
	else
		std::cerr << "[CGI] Failed to register fd " << fd << " with epoll" << std::endl;
}

void Server::_unregisterConnectionCGI(Connection &connection, CGIOperation op) noexcept
{
	int fd = connection.getCGIPipe(op);

	if (fd == -1)
	{
		std::cerr << "[CGI] (Server::closeCGI) no valid fd " << std::endl;
		return;
	}

	if (_poller.del(fd) == true)
	{
		std::cout << "[CGI] fd " << fd << " removed from EPOLL" << std::endl;
		_fd_to_connection.erase(fd);
		connection.closeCGIPipe(op);
	}
	else
		std::cerr << "[CGI] fd " << fd << " failed to remove from EPOLL" << std::endl;
}

void Server::_handleError( Connection & connection, int fd, bool isActiveCGI )
{
	std::cout << "IoState::Error" << std::endl;
	if (isActiveCGI)
	{
		std::cerr << "[io] EPOLLERR on CGI fd " << fd << std::endl;
		_unregisterConnectionCGI(connection, CGIOperation::WRITE);
		_unregisterConnectionCGI(connection, CGIOperation::READ);
	}
	else
	{
		std::cerr << "[io] EPOLLERR on fd " << fd << std::endl;
		closeConnection(fd);
	}
}

void Server::_handleClose( int fd, bool isActiveCGI )
{
	std::cout << "IoState::Closed" << std::endl;
	if (!isActiveCGI)
	{
		std::cout << "[io] EPOLLHUP on fd " << fd << std::endl;
		closeConnection(fd);
	}
}

void Server::_handleReceived( int fd )
{
	std::cout << "IoState::Received" << std::endl;

	modifyEvent(fd, EPOLLIN | EPOLLOUT);
}

void Server::_handleSent( Connection &connection, int fd, bool isActiveCGI )
{
	std::cout << "IoState::Sent" << std::endl;

	if (isActiveCGI)
	{
		_unregisterConnectionCGI(connection, CGIOperation::WRITE);
		_registerConnectionCGI(connection, CGIOperation::READ);
	}
	else
		modifyEvent(fd, EPOLLIN);
}

void Server::_handleCGIInit( Connection &connection )
{
	std::cout << "IoState::CGIInit" << std::endl;

	_registerConnectionCGI(connection, CGIOperation::WRITE);
	_pid_to_connection[connection.getCGIPID()] = &connection;
}

void Server::_handleCGIDone( Connection &connection )
{
	std::cout << "IoState::CGIDone" << std::endl;

	_unregisterConnectionCGI(connection, CGIOperation::READ);

	EventAction	action = connection.onCGIOutputReady();

	if (action == EventAction::EnableOutput)
		modifyEvent(connection.getFD(), EPOLLIN | EPOLLOUT);
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
			modifyEvent(connection->getFD(), EPOLLIN | EPOLLOUT);
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
