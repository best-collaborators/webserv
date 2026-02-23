#include "TransferEncodingChunkedParser.hpp"

TransferEncodingChunkedParser::TransferEncodingChunkedParser( ParseContext &parse_context ) :
_parse_context(parse_context) { }

bool TransferEncodingChunkedParser::_tryGetNewChunk( std::string &buffer )
{
	try {
		_chunk_size = std::stoull(buffer, nullptr, 16);
	}
	catch(const std::exception& e) { 
		std::cout << "[http-parser] Invalid size in transfer-encoding --> " << buffer << std::endl; 
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return false;
	}
	_parse_context.request.chunkHandler().setExpectedSize(_chunk_size);
	return true;
}

bool TransferEncodingChunkedParser::_isFinalChunk( std::string &buffer )
{
	if (_chunk_size == 0 && buffer.empty()) {

		_parse_context.request.set_status_code(HttpStatus::e_code::OK);
		_parse_context.request.chunkHandler().finalize();
		std::cout << "[http-parser] Received final chunk" << std::endl;
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isBad( std::string &buffer )
{
	if (_chunk_size > 0 && buffer.empty()) {
		std::cout << "[http-parser] Invalid chunk in transfer-encoding empty buffer" << std::endl; 
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isComplete( std::string &buffer )
{
	if (buffer.size() == _chunk_size) {
		std::cout << "COMPLETE CHUNK" << std::endl;
		//! PROBLEM
		_parse_context.request.get_body().append(buffer);
		_parse_context.request.chunkHandler().reset();
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
		std::cout << "AFTER COMPLETED CHUNK: " << std::endl;
		std::cout << buffer << std::endl;
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_consumeChunkSize(std::string &buffer, size_t bytes)
{
	buffer.append(_parse_context.raw_bits.substr(0, std::min(bytes, _parse_context.raw_bits.size())));
	_parse_context.raw_bits.erase(0, std::min(bytes, _parse_context.raw_bits.size()));

	std::cout << "bytes: " <<  bytes << std::endl;
	std::cout << "buffer size: " <<  buffer.size() << std::endl;
	std::cout << "raw bits: " <<  std::quoted(_parse_context.raw_bits) << std::endl;
	std::cout << "raw bits size: " <<  _parse_context.raw_bits.size() << std::endl;
	if (bytes >= _parse_context.raw_bits.size())
	{
		if (_parse_context.raw_bits.size() >= 2 && _parse_context.raw_bits.substr(0, 2) != "\r\n")
		{
			std::cout << "[http-parser] Invalid chunk in transfer-encoding no \\r\\n" << std::endl; 
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			_parse_context.request.chunkHandler().finalize();
			return false;
		}
		_parse_context.raw_bits.erase(0, 2);
	}
	return true;
}

void TransferEncodingChunkedParser::parse()
{
	std::string buffer = _parse_context.request.chunkHandler().getChunk();
	if (buffer.empty())
		buffer.append(RequestStringUtils::consume_next_line(_parse_context.raw_bits));

	// std::cout << "buffer: " << std::endl << std::quoted(buffer) << std::endl;
	// std::cout << "buffer_size: " << buffer.size() << std::endl;
	// std::cout << "RAW BITS: " << std::endl << _parse_context.raw_bits << std::endl;
	// std::cout << "_parse_context.request.chunk_handler().actualSize(): " << _parse_context.request.chunkHandler().getExpectedSize() << std::endl;
	// std::cout << "_parse_context.request.get_current_chunk_size_actual(): " << _parse_context.request.chunkHandler().actualSize() << std::endl;

	while (!buffer.empty())
	{
		if (!_parse_context.request.chunkHandler().getExpectedSize()) {
			if (!_tryGetNewChunk(buffer)) return ;
			buffer.clear();
			if (!_consumeChunkSize(buffer, _chunk_size)) return;
		}
		else {
			_chunk_size = _parse_context.request.chunkHandler().getExpectedSize();
			if (!_consumeChunkSize(buffer, _chunk_size - _parse_context.request.chunkHandler().actualSize())) return;
		}

		if (_isFinalChunk(buffer)) return ;
		if (_isComplete(buffer)) continue ;

		if (buffer.size() > _chunk_size) {
			std::cout << "[http-parser] Invalid chunk in transfer-encoding (too big)" << std::endl; 
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			_parse_context.request.chunkHandler().finalize();
			return ;
		}

		_parse_context.request.chunkHandler().setChunk(std::move(buffer));
		if (_parse_context.raw_bits.empty()) break;
	}
}
