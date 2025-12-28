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
		std::cerr << "gai error: " << gai_strerror(status) << std::endl;
		return 1;
	}

	int			listen_fd = -1;
	addrinfo *	address_node = nullptr;

	for (address_node = address_info; address_node != nullptr; address_node = address_node->ai_next)
	{
		if (address_node->ai_family == AF_INET)
		{
			std::cout << "Skipping IPv4 address_node." << std::endl;
			continue;
		}

		listen_fd = socket(address_node->ai_family, address_node->ai_socktype, address_node->ai_protocol);

		if (listen_fd == -1)
		{
			int	errsv = errno;
			std::cerr << "Socket failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			continue;
		}
		else
		{
			std::cout << "Created socket..." << std::endl;
		}

		int fcntl_status = fcntl(listen_fd, F_SETFL, O_NONBLOCK);

		if (fcntl_status == -1)
		{
			int	errsv = errno;
			std::cerr << "fcntl failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}
		else
		{
			std::cout << "Set listen_fd to non-blocking mode success" << std::endl;
		}

		int	reuse_address = 1;
		int reuse_address_status = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address));

		if (reuse_address_status == -1)
		{
			int	errsv = errno;
			std::cerr << "setsockopt SO_REUSEADDR failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}

		int	ipv6_only = 0;
		int ipv6_only_status = setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only));

		if (ipv6_only_status == -1)
		{
			int	errsv = errno;
			std::cerr << "setsockopt IPV6_V6ONLY failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}

		int	bind_status = bind(listen_fd, address_node->ai_addr, address_node->ai_addrlen);

		if (bind_status == -1)
		{
			int	errsv = errno;
			std::cerr << "Bind failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}
		else
		{
			std::cout << "Bound..." << std::endl;
		}

		break; //! Break after successfully bound to first address
	}

	freeaddrinfo(address_info);

	if (address_node == nullptr)
	{
		std::cerr << "Bind failed" << std::endl;
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
		std::cerr << "Listen failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
		close(listen_fd);
		return 1;
	}
	else
	{
		std::cout << "Listening..." << std::endl;
	}


	int epoll_fd = epoll_create1(0);

	if (epoll_fd == -1)
	{
		int	errsv = errno;
		std::cerr << "Epoll instance creation failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Epoll instance created with fd " << epoll_fd << "..." << std::endl;
	}

	epoll_event	event;

	event.events = EPOLLIN;
	event.data.fd = listen_fd;

	int epoll_ctl_status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);

	if (epoll_ctl_status == -1)
	{
		int	errsv = errno;
		std::cerr << "Epoll ctl on listen_fd failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Epoll EPOLL_CTL_ADD on listen_fd " << listen_fd << " success..." << std::endl;
	}

	int					client_fd;
	sockaddr_storage	client_address;
	socklen_t			client_address_size;
	epoll_event			triggered_events[MAX_TRIGGERED_EVENTS];

	while (true)
	{
		int	event_amount = epoll_wait(epoll_fd, triggered_events, MAX_TRIGGERED_EVENTS, TIMEOUT);
		if (event_amount == -1)
		{
			int	errsv = errno;
			std::cerr << "epoll_wait failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			continue;
		}

		std::cout << "epoll_wait return " << event_amount << " triggered events." << std::endl;
		for (int i = 0; i < event_amount; ++i)
		{
			std::cout << "event_amount " << event_amount << std::endl;
			std::cout << "triggered_events->data.fd " << triggered_events[i].data.fd << std::endl;
			std::cout << "listen_fd " << listen_fd << std::endl;
			if (triggered_events[i].data.fd == listen_fd)
			{
				std::cout << "WORK" << std::endl;
				client_address_size = static_cast<socklen_t>(sizeof(client_address));

				std::cout << "Before accept..." << std::endl;
				client_fd = accept(listen_fd, (sockaddr *)&client_address, &client_address_size);
				std::cout << "After accept..." << std::endl;

				if (client_fd == - 1)
				{
					int	errsv = errno;
					std::cerr << "Accept failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
					continue;
				}

				std::cout << "New connection on socket " << client_fd << std::endl;

				int fcntl_status = fcntl(client_fd, F_SETFL, O_NONBLOCK);

				if (fcntl_status == -1)
				{
					int	errsv = errno;
					std::cerr << "fcntl failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
					continue;
				}
				else
				{
					std::cout << "Set client_fd " << client_fd << " to non-blocking mode success" << std::endl;
				}

				event.events = EPOLLIN;
				event.data.fd = client_fd;

				int epoll_ctl_status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

				if (epoll_ctl_status == -1)
				{
					int	errsv = errno;
					std::cerr << "Epoll ctl on client_fd " << client_fd << " failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
					return 1;
				}
				else
				{
					std::cout << "Epoll EPOLL_CTL_ADD on client_fd " << client_fd << " success..." << std::endl;
				}
			}
			else
			{
				std::cout << "Before recv..." << std::endl;
				char	buffer[1000];
				int		received_bytes = recv(triggered_events[i].data.fd, buffer, sizeof(buffer), 0);
				std::cout << "After recv..." << std::endl;

				if (received_bytes == 0)
				{
					std::cout << "Socket " << triggered_events[i].data.fd << " closed." << std::endl;
				}
				else if (received_bytes < 0)
				{
					std::cerr << "Error occurred while recv..." << std::endl;
				}
				else
				{
					std::cout << "Content received successfully." << std::endl;
				}

				std::cout << "\nreceived_bytes: " << received_bytes << "\nbuffer_len: " << strlen(buffer) << "\n===============\n";
				std::string buff = buffer;
				std::cout << buff.substr(0, received_bytes) << "===============" << std::endl;
				received_bytes = recv(client_fd, buffer, sizeof(buffer), 0);
				// std::string body =
				// 	"<html>\n"
				// 	"<head><title>200 OK</title></head>\n"
				// 	"<body>\n"
				// 	"<center><h1>200 OK</h1></center>\n"
				// 	"</body>\n"
				// 	"</html>\n";

				// std::string headers =
				// 	"HTTP/1.1 200 OK\r\n"
				// 	"Content-Type: text/html\r\n"
				// 	"Content-Length: " + std::to_string(body.size()) + "\r\n"
				// 	"Connection: Close\r\n"
				// 	"\r\n";

				// std::string msg = headers + body;

				// std::cout << "Before send..." << std::endl;
				// send(triggered_events[i].data.fd, msg.c_str(), msg.length(), 0);
				// std::cout << "After send..." << std::endl;
			}
		}
	}

	close(listen_fd);
	return 0;
}
