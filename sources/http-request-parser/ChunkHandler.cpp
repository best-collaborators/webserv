#include "ChunkHandler.hpp"

ChunkHandler::ChunkHandler() : _expected_size(0), _received(false)  { }

const std::string& ChunkHandler::getChunk() const
{
	return _current_chunk;
}

size_t ChunkHandler::getExpectedSize() const
{
	return _expected_size;
}

bool ChunkHandler::isReceived() const
{
	return _received;
}

void ChunkHandler::markReceived()
{
	_received = true;
}

void ChunkHandler::setExpectedSize(size_t amount)
{
	_expected_size = amount;
}

size_t ChunkHandler::actualSize() const
{
	return _current_chunk.size();
}

void ChunkHandler::setChunk(std::string &&chunk)
{
	_current_chunk = std::move(chunk);
}

void ChunkHandler::reset()
{
	_current_chunk.clear();
	_expected_size = 0;
}

void ChunkHandler::finalize()
{
	reset();
	markReceived();
}

void ChunkHandler::append_to(std::string& body)
{
	body.append(_current_chunk);
	reset();
}