#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <iostream>

// socket, setsockopt, bind, listen, accept, recv, AF_UNSPEC, SOCK_STREAM, AF_INET, SOL_SOCKET, SO_REUSEADDR
# include <sys/socket.h>

// getaddrinfo, freeaddrinfo, gai_strerror, addrinfo, AI_PASSIVE, IPPROTO_IPV6, IPV6_V6ONLY
# include <netdb.h>

// strlen, strerror
# include <cstring>

// close
# include <unistd.h>

// fcntl, F_SETFL, O_NONBLOCK
# include <fcntl.h>

# define MAX_CONNECTIONS 10
# define PORT "3490"

#endif