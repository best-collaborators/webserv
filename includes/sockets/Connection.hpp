#pragma once

#include <iostream>
#include <sys/socket.h>

#include "IoState.hpp"

class Connection
{
private:
	static constexpr int	READ_BUFFER_SIZE = 0x8000;

	int			_fd;
	char		_temp_buffer[READ_BUFFER_SIZE];
	ssize_t		_read_bytes;
	ssize_t		_stored_bytes;
	std::string	_read_buffer;

	IoState		_saveToBuffer() noexcept;
	IoState		_checkSocketHealth() noexcept;

public:
	Connection();
	Connection( int fd );
	~Connection();

	IoState	receiveData() noexcept;
	IoState	sendData() noexcept;
};