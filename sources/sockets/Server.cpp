#include "../includes/sockets/Server.hpp"
#include "Poller.hpp"

Server::Server( std::string const & port ) : _listener(port), _poller()
{
	if (_poller.add(_listener.getFD(), EPOLLIN) == false)
	{
		throw std::system_error(errno, std::generic_category(), "[epoll] EPOLL_CTL_ADD listen_fd failed");
	}

	std::cout << "[epoll] Added listen_fd " << _listener.getFD() << " (EPOLLIN)." << std::endl;
}

void	Server::run()
{
	std::cout << "\n[accept] Waiting for connection..." << std::endl;

	while (true)
	{
		int event_count = _poller.wait();

		for (int i = 0; i < event_count; ++i)
		{
			epoll_event const &	event = _poller.getEvent(i);

			if (event.data.fd == _listener.getFD())
				acceptConnection();
			else
				handleEvent(event);
		}
	}
}

void	Server::acceptConnection()
{
	Socket	client = _listener.accept();
	int		client_fd = client.getFD();

	if (client_fd == -1)
		return;

	std::cout << "[accept] New connection fd " << client_fd << std::endl;

	if (!_poller.add(client_fd, EPOLLIN))
		return;

	_connections.emplace(client_fd, std::move(client));
	std::cout << "[epoll] Register new connection " << client_fd << " (EPOLLIN)." << std::endl;
}

void	Server::handleEvent( epoll_event const & event ) noexcept
{
	int	fd = event.data.fd;

	if (!_connections.count(fd))
		return;

	Connection &	connection = _connections.at(fd);

	IoState	state = connection.processEvents(event.events);

	switch (state)
	{
	case IoState::Error:
	case IoState::Closed:
		std::cout << "[io] EPOLLERR or EPOLLHUP on fd " << fd << std::endl;
		closeConnection(fd);
		break;
	case IoState::Received:
		modifyEvent(fd, EPOLLIN | EPOLLOUT);
		break;
	case IoState::Sent:
		modifyEvent(fd, EPOLLIN);
		break;
	default:
		break;
	}
}

void	Server::modifyEvent( int fd, uint32_t events ) noexcept
{
	if (_poller.mod(fd, events) == false)
	{
		closeConnection(fd);
	}
	else
	{
		std::cout << "[epoll] Updated fd " << fd << " to " << events << "." << std::endl;
	}
}

void	Server::closeConnection( int fd ) noexcept
{
	if (_poller.del(fd) == true)
	{
		_connections.erase(fd);

		std::cout << "[connection] Closed and removed fd " << fd << std::endl;
	}
}