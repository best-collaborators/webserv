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
			if (isConnectionEvent(triggered_events[i].data.fd))
			{
				int	connection_fd;

				if (!acceptNewConnection(connection_fd))
					continue;

				if (!registerNewConnection(connection_fd))
					continue;
			}
			else
			{
				if (!handleClientEvent(triggered_events[i]))
					continue;
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

bool	EventLoop::isConnectionEvent( int fd ) const
{
	return fd == _listen_fd;
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

bool	EventLoop::registerNewConnection( int & connection_fd )
{
	epoll_event	client_event {};
	client_event.events = EPOLLIN;
	client_event.data.fd = connection_fd;

	int epoll_ctl_status = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, connection_fd, &client_event);

	if (epoll_ctl_status == -1)
	{
		std::cerr << "[epoll] EPOLL_CTL_ADD connection_fd " << connection_fd << " failed (" << errno << "): " << strerror(errno) << std::endl;
		return false;
	}

	std::cout << "[epoll] Register new connection " << connection_fd << " (EPOLLIN)." << std::endl;
	return true;
}

bool	EventLoop::handleClientEvent( epoll_event & event )
{
	if (event.events & EPOLLIN)
	{
		std::cout << "\n[io] EPOLLIN triggered for fd " << event.data.fd << std::endl;
		std::cout << "[io] recv() starting..." << std::endl;
		char	buffer[READ_BUFFER_SIZE];
		int		received_bytes = recv(event.data.fd, buffer, sizeof(buffer), 0);
		std::cout << "[io] recv() completed." << std::endl;

		if (received_bytes == 0)
		{
			std::cout << "[io] Peer closed fd " << event.data.fd << "." << std::endl;
			close(event.data.fd);
			return false;
		}
		else if (received_bytes < 0)
		{
			// n < 0: Treat this as a "Spurious Wakeup" or "Wait State" and return to the loop.
			// Note: Since the socket was marked readable, this shouldn't happen often. Without errno, you have to assume the connection is still alive but temporarily unavailable, or treat it as a fatal error depending on your tolerance.
			std::cerr << "[io] recv() error." << std::endl;
			return false;
		}
		else if (received_bytes > 0)
		{
			if (received_bytes < READ_BUFFER_SIZE)
			{
				std::cout << "[io] Request received (complete)." << std::endl;
			}
			else if (received_bytes == READ_BUFFER_SIZE)
			{
				std::cout << "[io] Request received (partial buffer)." << std::endl;
			}
			std::cout << "\n[io] received_bytes: " << received_bytes << "\n[io] buffer_len: " << strlen(buffer) << "\n===============\n";
			std::string buff = buffer;
			std::cout << buff.substr(0, received_bytes) << "===============" << std::endl;

			epoll_event	ev {};

			ev.events = EPOLLIN | EPOLLOUT;
			ev.data.fd = event.data.fd;

			int mod_status = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, event.data.fd, &ev);
			if (mod_status == -1)
			{
				int	errsv = errno;
				std::cerr << "[epoll] EPOLL_CTL_MOD to EPOLLIN|EPOLLOUT for fd " << event.data.fd << " failed (" << errsv << "): " << strerror(errsv) << std::endl;
				// return 1; //! Clean fds
				return false;
			}
			else
			{
				std::cout << "[epoll] Updated fd " << event.data.fd << " to EPOLLIN|EPOLLOUT." << std::endl;
			}
		}
	}
	if (event.events & EPOLLOUT)
	{
		std::cout << "\n[io] EPOLLOUT triggered for fd " << event.data.fd << std::endl;
		std::string body =
			"<html>\n"
			"<head><title>200 OK</title></head>\n"
			"<body>\n"
			"<center><h1>200 OK</h1></center>\n"
			"</body>\n"
			"</html>\n";

		std::string headers =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"Connection: keep-alive\r\n"
			"\r\n";

		std::string msg = headers + body;

		std::cout << "[io] send() starting..." << std::endl;
		ssize_t sent_bytes = send(event.data.fd, msg.c_str(), msg.length(), 0);
		std::cout << "[io] send() completed." << std::endl;

		if (sent_bytes == static_cast<ssize_t>(msg.length()))
		{
			std::cout << "[io] Response sent (complete)." << std::endl;
		}
		else if (sent_bytes <= 0)
		{
			std::cout << "[io] Send failed or would block." << std::endl;
		}
		else if (sent_bytes < static_cast<ssize_t>(msg.length()))
		{
			std::cout << "[io] Response sent partially." << std::endl;
		}

		if (sent_bytes == static_cast<ssize_t>(msg.length()))
		{
			epoll_event	ev {};

			ev.events = EPOLLIN;
			ev.data.fd = event.data.fd;

			int mod_status = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, event.data.fd, &ev);
			if (mod_status == -1)
			{
				int	errsv = errno;
				std::cerr << "[epoll] EPOLL_CTL_MOD to EPOLLIN for fd " << event.data.fd << " failed (" << errsv << "): " << strerror(errsv) << std::endl;
				// return 1; //! Clean fds
				return false;
			}
			else
			{
				std::cout << "[epoll] Updated fd " << event.data.fd << " to EPOLLIN only." << std::endl;
			}
		}
	}
	if (event.events & EPOLLERR)
	{
		std::cout << "[io] EPOLLERR on fd " << event.data.fd << std::endl;
		close(event.data.fd);
	}
	if (event.events & EPOLLHUP)
	{
		std::cout << "[io] EPOLLHUP on fd " << event.data.fd << std::endl;
		close(event.data.fd);
	}
	return true;
}
