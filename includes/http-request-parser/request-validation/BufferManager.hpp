#ifndef BUFFER_MANAGER
#define BUFFER_MANAGER

#include <iostream>

class BufferManager
{
private:
	static constexpr int	READ_BUFFER_SIZE = 32768;

	std::string	_read_buffer;
	char		_recv_buffer[READ_BUFFER_SIZE];

public:
	BufferManager() = default;
	~BufferManager() = default;

	char				*getRecvBuffer() noexcept;
	size_t				getReceiveBufferSize() noexcept;
	void				append(size_t bytes) noexcept;
	void				consume(size_t bytes) noexcept;
	void				clear() noexcept;
	std::string			&getBuffer() noexcept;
};

#endif /* BUFFER_MANAGER */
