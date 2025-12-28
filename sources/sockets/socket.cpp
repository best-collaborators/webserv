#include "../includes/sockets/socket.hpp"

int	main( void )
{
	int			status;
	addrinfo	hints {};
	addrinfo *	address_info = nullptr;

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, PORT, &hints, &address_info);
	if (status != 0)
	{
		std::cerr << "[gai] Resolution failed: " << gai_strerror(status) << std::endl;
		return 1;
	}

	int			listen_fd = -1;
	addrinfo *	address_node = nullptr;

	for (address_node = address_info; address_node != nullptr; address_node = address_node->ai_next)
	{
		if (address_node->ai_family == AF_INET)
		{
			std::cout << "[listen] Skipping IPv4 candidate." << std::endl;
			continue;
		}

		listen_fd = socket(address_node->ai_family, address_node->ai_socktype, address_node->ai_protocol);

		if (listen_fd == -1)
		{
			int	errsv = errno;
			std::cerr << "[listen] socket() failed (" << errsv << "): " << strerror(errsv) << std::endl;
			continue;
		}
		else
		{
			std::cout << "[listen] Socket created." << std::endl;
		}

		int fcntl_status = fcntl(listen_fd, F_SETFL, O_NONBLOCK);

		if (fcntl_status == -1)
		{
			int	errsv = errno;
			std::cerr << "[listen] fcntl(O_NONBLOCK) failed (" << errsv << "): " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}
		else
		{
			std::cout << "[listen] listen_fd set to non-blocking." << std::endl;
		}

		int	reuse_address = 1;
		int reuse_address_status = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address));

		if (reuse_address_status == -1)
		{
			int	errsv = errno;
			std::cerr << "[listen] setsockopt(SO_REUSEADDR) failed (" << errsv << "): " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}

		int	ipv6_only = 0;
		int ipv6_only_status = setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));

		if (ipv6_only_status == -1)
		{
			int	errsv = errno;
			std::cerr << "[listen] setsockopt(IPV6_V6ONLY) failed (" << errsv << "): " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}

		int	bind_status = bind(listen_fd, address_node->ai_addr, address_node->ai_addrlen);

		if (bind_status == -1)
		{
			int	errsv = errno;
			std::cerr << "[listen] bind() failed (" << errsv << "): " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}
		else
		{
			std::cout << "[listen] Socket bound." << std::endl;
		}

		break; //! Break after successfully bound to first address
	}

	freeaddrinfo(address_info);

	if (address_node == nullptr)
	{
		std::cerr << "[listen] Bind failed for all addresses." << std::endl;
		if (listen_fd != -1)
		{
			close(listen_fd);
		}
		return 1;
	}

	int	listen_status = listen(listen_fd, MAX_CONNECTIONS);

	if (listen_status == -1)
	{
		int	errsv = errno;
		std::cerr << "[listen] listen() failed (" << errsv << "): " << strerror(errsv) << std::endl;
		close(listen_fd);
		return 1;
	}
	else
	{
		std::cout << "\n[listen] Listening on port " << PORT << "..." << std::endl;
	}

	int epoll_fd = epoll_create1(0);

	if (epoll_fd == -1)
	{
		int	errsv = errno;
		std::cerr << "[epoll] epoll_create1 failed (" << errsv << "): " << strerror(errsv) << std::endl;
		return 1;
	}
	else
	{
		std::cout << "[epoll] Created instance fd=" << epoll_fd << "." << std::endl;
	}

	epoll_event	event;

	event.events = EPOLLIN;
	event.data.fd = listen_fd;

	int epoll_ctl_status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);

	if (epoll_ctl_status == -1)
	{
		int	errsv = errno;
		std::cerr << "[epoll] EPOLL_CTL_ADD listen_fd failed (" << errsv << "): " << strerror(errsv) << std::endl;
		close(listen_fd);
		close(epoll_fd);
		return 1;
	}
	else
	{
		std::cout << "[epoll] Added listen_fd " << listen_fd << " (EPOLLIN)." << std::endl;
	}

	int					client_fd;
	sockaddr_storage	client_address {};
	socklen_t			client_address_size {};
	epoll_event			triggered_events[MAX_TRIGGERED_EVENTS] {};

	while (true)
	{
		int	event_amount = epoll_wait(epoll_fd, triggered_events, MAX_TRIGGERED_EVENTS, TIMEOUT);
		if (event_amount == -1)
		{
			int	errsv = errno;
			std::cerr << "[epoll] epoll_wait failed (" << errsv << "): " << strerror(errsv) << std::endl;
			continue;
		}

		std::cout << "\n[epoll] epoll_wait returned " << event_amount << " event(s)." << std::endl;
		for (int i = 0; i < event_amount; ++i)
		{
			if (triggered_events[i].data.fd == listen_fd)
			{
				client_address_size = static_cast<socklen_t>(sizeof(client_address));

				std::cout << "\n[accept] Waiting for connection..." << std::endl;
				client_fd = accept(listen_fd, (sockaddr *)&client_address, &client_address_size);
				std::cout << "[accept] accept() returned." << std::endl;

				if (client_fd == - 1)
				{
					int	errsv = errno;
					std::cerr << "[accept] Failed (" << errsv << "): " << strerror(errsv) << std::endl;
					continue;
				}

				std::cout << "[accept] New client fd=" << client_fd << std::endl;

				int fcntl_status = fcntl(client_fd, F_SETFL, O_NONBLOCK);

				if (fcntl_status == -1)
				{
					int	errsv = errno;
					std::cerr << "[accept] fcntl(O_NONBLOCK) failed (" << errsv << "): " << strerror(errsv) << std::endl;
					continue;
				}
				else
				{
					std::cout << "[accept] client_fd " << client_fd << " set to non-blocking." << std::endl;
				}

				epoll_event	client_event {};
				client_event.events = EPOLLIN;
				client_event.data.fd = client_fd;

				int epoll_ctl_status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);

				if (epoll_ctl_status == -1)
				{
					int	errsv = errno;
					std::cerr << "[epoll] EPOLL_CTL_ADD client_fd " << client_fd << " failed (" << errsv << "): " << strerror(errsv) << std::endl;
					continue;
				}
				else
				{
					std::cout << "[epoll] Added client_fd " << client_fd << " (EPOLLIN)." << std::endl;
				}
			}
			else
			{
				if (triggered_events[i].events & EPOLLIN)
				{
					std::cout << "\n[io] EPOLLIN triggered for fd " << triggered_events[i].data.fd << std::endl;
					std::cout << "[io] recv() starting..." << std::endl;
					char	buffer[READ_BUFFER_SIZE];
					int		received_bytes = recv(triggered_events[i].data.fd, buffer, sizeof(buffer), 0);
					std::cout << "[io] recv() completed." << std::endl;

					if (received_bytes == 0)
					{
						std::cout << "[io] Peer closed fd " << triggered_events[i].data.fd << "." << std::endl;
						close(triggered_events[i].data.fd);
					}
					else if (received_bytes < 0)
					{
						// n < 0: Treat this as a "Spurious Wakeup" or "Wait State" and return to the loop.
						// Note: Since the socket was marked readable, this shouldn't happen often. Without errno, you have to assume the connection is still alive but temporarily unavailable, or treat it as a fatal error depending on your tolerance.
						std::cerr << "[io] recv() error." << std::endl;
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
						ev.data.fd = triggered_events[i].data.fd;

						int mod_status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, triggered_events[i].data.fd, &ev);
						if (mod_status == -1)
						{
							int	errsv = errno;
							std::cerr << "[epoll] EPOLL_CTL_MOD to EPOLLIN|EPOLLOUT for fd " << triggered_events[i].data.fd << " failed (" << errsv << "): " << strerror(errsv) << std::endl;
							return 1; //! Clean fds
						}
						else
						{
							std::cout << "[epoll] Updated fd " << triggered_events[i].data.fd << " to EPOLLIN|EPOLLOUT." << std::endl;
						}
					}
				}
				if (triggered_events[i].events & EPOLLOUT)
				{
					std::cout << "\n[io] EPOLLOUT triggered for fd " << triggered_events[i].data.fd << std::endl;
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
					ssize_t sent_bytes = send(triggered_events[i].data.fd, msg.c_str(), msg.length(), 0);
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
						ev.data.fd = triggered_events[i].data.fd;
		
						int mod_status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, triggered_events[i].data.fd, &ev);
						if (mod_status == -1)
						{
							int	errsv = errno;
							std::cerr << "[epoll] EPOLL_CTL_MOD to EPOLLIN for fd " << triggered_events[i].data.fd << " failed (" << errsv << "): " << strerror(errsv) << std::endl;
							return 1; //! Clean fds
						}
						else
						{
							std::cout << "[epoll] Updated fd " << triggered_events[i].data.fd << " to EPOLLIN only." << std::endl;
						}
					}
				}
				if (triggered_events[i].events & EPOLLERR)
				{
					std::cout << "[io] EPOLLERR on fd " << triggered_events[i].data.fd << std::endl;
					close(triggered_events[i].data.fd);
				}
				if (triggered_events[i].events & EPOLLHUP)
				{
					std::cout << "[io] EPOLLHUP on fd " << triggered_events[i].data.fd << std::endl;
					close(triggered_events[i].data.fd);
				}
			}
		}
	}

	close(listen_fd);
	return 0;
}
