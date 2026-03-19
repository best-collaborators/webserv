#include "TransferEncodingChunkedParser.hpp"

std::string TransferEncodingChunkedParser::old_raw_bits = "";

TransferEncodingChunkedParser::TransferEncodingChunkedParser( ParseContext &parse_context ) :
_parse_context(parse_context) { }

bool TransferEncodingChunkedParser::_tryGetNewChunk( std::string &buffer )
{
	try {
		_chunk_size = std::stoull(buffer, nullptr, 16);
	}
	catch(const std::exception& e) {
		Log::error("Invalid size in transfer-encoding --> ", "http-parser");
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return false;
	}
	_parse_context.request.chunkHandler().setExpectedSize(_chunk_size);
	return true;
}

bool TransferEncodingChunkedParser::_isFinalChunk( std::string &buffer )
{
	if (_parse_context.request.get_body().empty() && _chunk_size > _parse_context.request.getFile().getMaxBodySize()) {
		_parse_context.request.set_status_code(HttpStatus::e_code::CONTENT_TOO_LARGE);
		std::cout << "[http-parser] Body is too big in trans_parse_context.raw_bits.size()fer-encoding" << std::endl;
		_parse_context.request.chunkHandler().finalize();
		return true;
	}

	if (_parse_context.request.get_body().size() > _parse_context.request.getFile().getMaxBodySize()) {
		_parse_context.request.set_status_code(HttpStatus::e_code::CONTENT_TOO_LARGE);
		std::cout << "[http-parser] Body is too big in transfer-encoding" << std::endl;
		_parse_context.request.chunkHandler().finalize();
		return true;
	}

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
		// std::cout << _parse_context.request.get_body().size() << std::endl;
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_consumeChunkSize(std::string &buffer)
{
	old_raw_bits = _parse_context.raw_bits;

	size_t total_size = _parse_context.request.chunkHandler().getExpectedSize();
	auto pos = _parse_context.raw_bits.find("\r\n");
	if (pos == std::string::npos) {
		pos = _parse_context.raw_bits.size();
	}
	size_t count_characters = pos + buffer.size();
	if (total_size < count_characters)
	{
		std::cout << "[http-parser] Invalid chunk in transfer-encoding no \\r\\n" << std::endl; 
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return false;
	}
	buffer.append(_parse_context.raw_bits.substr(0, pos));
	_parse_context.raw_bits.erase(0, pos);

	std::cout << "raw bits size: " <<  _parse_context.raw_bits.size() << std::endl;
	return true;
}

// 4\r\n
// eeeeeeee\r\n
// 5\r\n
// eeeee\r\n
// 0\r\n
// \r\n

// 'e' <repeats 16391 times>, "8000\r\n", 'e' <repeats 16371 times>

// 'e' <repeats 32762 times>
// 'e' <repeats 32768 times>, "\r\n8000\r\n", 'e' <repeats 32754 times>
void TransferEncodingChunkedParser::parse()
{
	// std::cout << "buffer: " << std::endl << std::quoted(buffer) << std::endl;
	// std::cout << "buffer_size: " << buffer.size() << std::endl;
	// std::cout << "RAW BITS: " << std::endl << _parse_context.raw_bits << std::endl;
	// std::cout << "_parse_context.request.chunk_handler().actualSize(): " << _parse_context.request.chunkHandler().getExpectedSize() << std::endl;
	// std::cout << "_parse_context.request.get_current_chunk_size_actual(): " << _parse_context.request.chunkHandler().actualSize() << std::endl;

	while (true)
	{
		_chunk_size = _parse_context.request.chunkHandler().getExpectedSize();
		if (_chunk_size == 0) {
			auto pos = _parse_context.raw_bits.find("\r\n");
			if (pos == std::string::npos) {
				break;
			}

			std::string line = _parse_context.raw_bits.substr(0, pos);
			_parse_context.raw_bits.erase(0, pos + 2);
			if (!_tryGetNewChunk(line)) return ;

			if (_chunk_size == 0) {
				_parse_context.request.set_status_code(HttpStatus::e_code::OK);
				_parse_context.request.chunkHandler().finalize();
				std::cout << "[http-parser] Received final chunk" << std::endl;
				return ;
			}
		}

		if (_parse_context.raw_bits.size() < _chunk_size + 2)
			break;


		std::string buffer = _parse_context.raw_bits.substr(0, _chunk_size);
		_parse_context.raw_bits.erase(0, _chunk_size);

		if (_parse_context.raw_bits.substr(0, 2) != "\r\n")
		{
			std::cout << "[http-parser] Invalid chunk in transfer-encoding no \\r\\n" << std::endl; 
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			_parse_context.request.chunkHandler().finalize();
			return ;
		}
		_parse_context.raw_bits.erase(0, 2);
		_parse_context.request.chunkHandler().reset();
		_chunk_size = 0;
	}
}
