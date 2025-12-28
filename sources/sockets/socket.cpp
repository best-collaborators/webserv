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

		int	enable_reuse_address = 1;
		int set_reuse_status = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse_address, sizeof(enable_reuse_address));

		if (set_reuse_status == -1)
		{
			int	errsv = errno;
			std::cerr << "setsockopt failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
			close(listen_fd);
			continue;
		}

		int	disable_only_ipv6 = 0;
		int disable_only_status = setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &disable_only_ipv6, sizeof(disable_only_ipv6));

		if (disable_only_status == -1)
		{
			int	errsv = errno;
			std::cerr << "setsockopt failed with error code " << errsv << ": " << strerror(errsv) << std::endl;
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
		close(listen_fd);
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

	close(listen_fd);
	return 0;
}
