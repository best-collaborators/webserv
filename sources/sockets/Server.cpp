#include "../includes/sockets/Server.hpp"
#include "../includes/sockets/SocketUtils.hpp"
#include "Poller.hpp"

Server::Server( std::string const & port ) : _listener(port), _poller()
{
	if (_poller.add(_listener.getFD(), EPOLLIN) == false)
	{
		throw std::system_error(errno, std::generic_category(), "[epoll] EPOLL_CTL_ADD listen_fd failed");
	}

	std::cout << "[epoll] Added listen_fd " << _listener.getFD() << " (EPOLLIN)." << std::endl;
}

Server::~Server()
{}

void	Server::run()
{
	std::cout << "\n[accept] Waiting for connection..." << std::endl;

	while (true)
	{
		int event_count = _poller.wait();

		for (int i = 0; i < event_count; ++i)
		{
			epoll_event const & event = _poller.getEvent(i);

			if (event.data.fd == _listener.getFD())
			{
				int	connection_fd;

				if (!acceptNewConnection(connection_fd))
					continue;

				registerNewConnection(connection_fd);
			}
			else
			{
				handleClientEvent(event);
			}
		}
	}
}

bool	Server::acceptNewConnection( int & connection_fd )
{
	sockaddr_storage	connection_address {};
	socklen_t			connection_address_size {};

	connection_fd = accept(_listener.getFD(), (sockaddr *)&connection_address, &connection_address_size);
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

void	Server::registerNewConnection( int & fd ) noexcept
{
	if (!_poller.add(fd, EPOLLIN))
	{
		close(fd);
	}
	else
	{
		Connection	connection(fd);

		connections.insert({ fd, connection });
		std::cout << "[epoll] Register new connection " << fd << " (EPOLLIN)." << std::endl;
	}
}

void	Server::handleClientEvent( epoll_event const & event ) noexcept
{
	int	fd = event.data.fd;

	if (event.events & EPOLLIN)
	{
		IoState state = connections.at(fd).receiveData();

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
		IoState state = connections.at(fd).sendData();

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

void	Server::registerEventToReadWrite( int fd ) noexcept
{
	if (_poller.mod(fd, EPOLLIN | EPOLLOUT) == false)
	{
		closeConnection(fd);
	}
	else
	{
		std::cout << "[epoll] Updated fd " << fd << " to EPOLLIN | EPOLLOUT." << std::endl;
	}
}

void	Server::registerEventToReadOnly( int fd ) noexcept
{
	if (_poller.mod(fd, EPOLLIN) == false)
	{
		closeConnection(fd);
	}
	else
	{
		std::cout << "[epoll] Updated fd " << fd << " to EPOLLIN only." << std::endl;
	}
}

void	Server::closeConnection( int fd ) noexcept
{
	if (fd != -1)
	{
		if (_poller.del(fd))
		{
			close(fd);
			connections.erase(fd);
			std::cout << "[connection] Closed and removed fd " << fd << std::endl;
		}
	}
}