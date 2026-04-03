#ifndef BUFFER_MANAGER
#define BUFFER_MANAGER

#include <iostream>
#include <vector>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>

class BufferManager
{
private:
	static constexpr int	READ_BUFFER_SIZE = 32768;

	std::string	_read_buffer;
	char *_recv_buffer;

public:
	BufferManager();
	~BufferManager();

	BufferManager &operator=(const BufferManager &other);

	char				*getRecvBuffer() noexcept;
	size_t				getReceiveBufferSize() noexcept;
	void				append(size_t bytes) noexcept;
	void				consume(size_t bytes) noexcept;
	void				clear() noexcept;
	std::string			&getBuffer() noexcept;
};

#endif /* BUFFER_MANAGER */
