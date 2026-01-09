#include "../includes/sockets/EventLoop.hpp"
#include "../includes/sockets/SocketUtils.hpp"

EventLoop::EventLoop( int listen_fd ) : _listen_fd(listen_fd)
{}

EventLoop::~EventLoop()
{
	SocketUtils::safeCloseFD(_epoll_fd);
}

void	EventLoop::init()
{
	createEpollInstance();
	registerListenSocket();
}

void	EventLoop::monitor()
{
	std::cout << "\n[accept] Waiting for connection..." << std::endl;

	while (true)
	{
		int	event_count;

		if (!monitorEvents(event_count))
			continue;

		for (int i = 0; i < event_count; ++i)
		{
			if (triggered_events[i].data.fd == _listen_fd)
			{
				int	connection_fd;

				if (!acceptNewConnection(connection_fd))
					continue;

				registerNewConnection(connection_fd);
			}
			else
			{
				handleClientEvent(triggered_events[i]);
			}
		}
	}
}

void	EventLoop::createEpollInstance()
{
	int status = epoll_create1(0);

	SocketUtils::checkStatus(status, "[epoll] epoll_create failed");

	_epoll_fd = status;

	std::cout << "[epoll] Created instance fd=" << _epoll_fd << "." << std::endl;
}

void	EventLoop::registerListenSocket()
{
	epoll_event	event;

	event.events = EPOLLIN;
	event.data.fd = _listen_fd;

	int status = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listen_fd, &event);

	SocketUtils::checkStatus(status, "[epoll] EPOLL_CTL_ADD listen_fd failed");

	std::cout << "[epoll] Added _listen_fd " << _listen_fd << " (EPOLLIN)." << std::endl;
}

int		EventLoop::monitorEvents( int & event_count )
{
	event_count = epoll_wait(_epoll_fd, triggered_events, MAX_TRIGGERED_EVENTS, TIMEOUT);

	if (event_count == -1)
	{
		std::cerr << "[epoll] epoll_wait failed (" << errno << "): " << strerror(errno) << std::endl;
		return false;
	}

	std::cout << "\n[epoll] epoll_wait returned " << event_count << " event(s)." << std::endl;
	return true;
}

bool	EventLoop::acceptNewConnection( int & connection_fd )
{
	sockaddr_storage	connection_address {};
	socklen_t			connection_address_size {};

	connection_fd = accept(_listen_fd, (sockaddr *)&connection_address, &connection_address_size);
	std::cout << "[accept] accept() returned." << std::endl;

	if (connection_fd == - 1)
	{
		std::cerr << "[accept] Failed (" << errno << "): " << strerror(errno) << std::endl;
		return false;
	}

	int status = fcntl(connection_fd, F_SETFL, O_NONBLOCK);

	if (status == -1)
	{
		std::cerr << "[accept] fcntl(O_NONBLOCK) failed (" << errno << "): " << strerror(errno) << std::endl;
		return false;
	}

	std::cout << "[accept] New connection fd " << connection_fd << std::endl;
	return true;
}

void	EventLoop::registerNewConnection( int & fd ) noexcept
{
	epoll_event	client_event {};

	client_event.events = EPOLLIN;
	client_event.data.fd = fd;

	int status = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &client_event);

	if (status == -1)
	{
		std::cerr << "[epoll] EPOLL_CTL_ADD fd " << fd << " failed (" << errno << "): " << strerror(errno) << std::endl;
		close(fd);
	}
	else
	{
		Connection	connection(fd);

		connections.insert({ fd, connection });
		std::cout << "[epoll] Register new connection " << fd << " (EPOLLIN)." << std::endl;
	}
}

void	EventLoop::handleClientEvent( epoll_event & event ) noexcept
{
	int	fd = event.data.fd;

	if (event.events & EPOLLIN)
	{
		IoState state = connections.find(fd).receiveData();

		if (state == IoState::Error || state == IoState::Closed)
		{
			closeConnection(fd);
			return;
		}
		else if (state == IoState::Ready)
		{
			registerEventToReadWrite(fd);
		}
	}
	if (event.events & EPOLLOUT)
	{
		IoState state = connections.find(fd).sendData();

		if (state == IoState::Error || state == IoState::Closed)
		{
			closeConnection(fd);
			return;
		}
		else if (state == IoState::Ready)
		{
			registerEventToReadOnly(fd);
		}
	}
	if (event.events & EPOLLERR || event.events & EPOLLHUP)
	{
		std::cout << "[io] EPOLLERR or EPOLLHUP on fd " << event.data.fd << std::endl;
		closeConnection(event.data.fd);
	}
}

void	EventLoop::registerEventToReadWrite( int fd ) noexcept
{
	epoll_event	event {};

	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = fd;

	int status = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event);

	if (status == -1)
	{
		std::cerr << "[epoll] EPOLL_CTL_MOD to EPOLLIN|EPOLLOUT for fd " << fd << " failed (" << errno << "): " << strerror(errno) << std::endl;
		closeConnection(fd);
	}
	else
	{
		std::cout << "[epoll] Updated fd " << fd << " to EPOLLIN|EPOLLOUT." << std::endl;
	}
}

void	EventLoop::registerEventToReadOnly( int fd ) noexcept
{
	epoll_event	event {};

	event.events = EPOLLIN;
	event.data.fd = fd;

	int	status = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event);

	if (status == -1)
	{
		std::cerr << "[epoll] EPOLL_CTL_MOD to EPOLLIN for fd " << fd << " failed (" << errno << "): " << strerror(errno) << std::endl;
		closeConnection(fd);
	}
	else
	{
		std::cout << "[epoll] Updated fd " << fd << " to EPOLLIN only." << std::endl;
	}
}

void	EventLoop::closeConnection( int fd ) noexcept
{
	if (fd != -1)
	{
		int	status = epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
		
		if (status == -1)
		{
			std::cerr << "[epoll] EPOLL_CTL_DELL for fd " << fd << " failed (" << errno << "): " << strerror(errno) << std::endl;
		}
		else
		{
			close(fd);
			connections.erase(fd);
			std::cout << "[connection] Closed and removed fd " << fd << std::endl;
		}
	}
}