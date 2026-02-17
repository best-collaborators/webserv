#include "TransferEncodingChunkedParser.hpp"

TransferEncodingChunkedParser::TransferEncodingChunkedParser( ParseContext &parse_context ) :
_parse_context(parse_context) { }

bool TransferEncodingChunkedParser::_tryGetNewChunk( std::string &buffer )
{
	std::string hex = RegexMatcher::get_regex_value(buffer, HttpRegexPatterns::HEX_VALUE());
	try {
		_chunk_size = std::stoull(hex, nullptr, 16);
	}
	catch(const std::exception& e) { 
		std::cout << "[http-parser] Invalid size in transfer-encoding --> " << hex << std::endl; 
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return false;
	}
	_parse_context.request.chunkHandler().setExpectedSize(_chunk_size);

	buffer = RequestStringUtils::consume_next_line(_parse_context.raw_bits);
	return true;
}

bool TransferEncodingChunkedParser::_isFinalChunk( std::string &buffer )
{
	if (_chunk_size == 0 && buffer.empty() && _parse_context.raw_bits.empty()) {
		_parse_context.request.set_status_code(HttpStatus::e_code::OK);
		_parse_context.request.chunkHandler().finalize();
		std::cout << "[http-parser] Received final chunk" << std::endl;
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isBad( std::string &buffer )
{
	if (_chunk_size > 0 && buffer.empty() && !_parse_context.raw_bits.empty()) {
		std::cout << "[http-parser] Invalid chunk in transfer-encoding" << std::endl; 
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
		return true;
	}
	return false;
}

void TransferEncodingChunkedParser::parse()
{
	std::string buffer = _parse_context.request.chunkHandler().getChunk();
	buffer.append(RequestStringUtils::consume_next_line(_parse_context.raw_bits));

	// std::cout << "RAW BITS: " << std::endl << _parse_context.raw_bits << std::endl;
	// std::cout << "buffer: " << std::endl << std::quoted(buffer) << std::endl;
	// std::cout << "_parse_context.request.chunk_handler().actualSize(): " << _parse_context.request.chunkHandler().getExpectedSize() << std::endl;
	// std::cout << "_parse_context.request.get_current_chunk_size_actual(): " << _parse_context.request.chunkHandler().actualSize() << std::endl;

	while (!buffer.empty())
	{
		if (!_parse_context.request.chunkHandler().getExpectedSize()) {
			if (!_tryGetNewChunk(buffer)) return ;
		}
		else {
			std::cout << "DEBUG" << std::endl;
			_chunk_size = _parse_context.request.chunkHandler().getExpectedSize();
		}

		if (_isFinalChunk(buffer) || _isBad(buffer)) return ;
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
