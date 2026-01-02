#pragma once

#include <unistd.h>

#include <string>
#include <system_error>

class SocketUtils
{
public:
	SocketUtils() = delete;

	static void	checkStatus( int status, std::string const & message );
	static void	safeCloseFD( int & fd ) noexcept;

};
