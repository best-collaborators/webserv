#include "BufferManager.hpp"

char *BufferManager::getRecvBuffer() noexcept
{
	return _recv_buffer;
}

size_t BufferManager::getReceiveBufferSize() noexcept
{
	return _read_buffer.size();
}

void BufferManager::append(size_t bytes) noexcept
{
	_read_buffer.append(_recv_buffer, bytes);
}

void BufferManager::consume(size_t bytes) noexcept
{
	_read_buffer.erase(bytes);
}

void BufferManager::clear() noexcept
{
	_read_buffer.clear();
}

std::string &BufferManager::getBuffer() noexcept
{
	return _read_buffer;
}
