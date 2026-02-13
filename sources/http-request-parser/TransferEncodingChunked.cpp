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
		_parse_context.request.set_status_code(400);
		_parse_context.request.set_is_chunk_received(true);
		_parse_context.request.set_current_chunk("");
		_parse_context.request.set_chunk_size(0);
		return false;
	}
	_parse_context.request.set_chunk_size(_chunk_size);

	buffer = RequestStringUtils::consume_next_line(_parse_context.raw_bits);
	return true;
}

bool TransferEncodingChunkedParser::_isFinalChunk( std::string &buffer )
{
	if (_chunk_size == 0 && buffer.empty() && _parse_context.raw_bits.empty()) {
		_parse_context.request.set_status_code(200);
		_parse_context.request.set_is_chunk_received(true);
		_parse_context.request.set_current_chunk("");
		_parse_context.request.set_chunk_size(0);
		std::cout << "[http-parser] Received final chunk" << std::endl; 
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isBad( std::string &buffer )
{
	if ((_chunk_size > 0 && buffer.empty())) {
		std::cout << "[http-parser] Invalid chunk in transfer-encoding" << std::endl; 
		_parse_context.request.set_status_code(400);
		_parse_context.request.set_is_chunk_received(true);
		_parse_context.request.set_current_chunk("");
		_parse_context.request.set_chunk_size(0);
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isComplete( std::string &buffer )
{
	if (buffer.size() == _chunk_size) {
		std::cout << "COMPLETE CHUNK" << std::endl;
		_parse_context.request.append_body_value(buffer);
		_parse_context.request.set_current_chunk("");
		_parse_context.request.set_chunk_size(0);
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
		return true;
	}
	return false;
}

void TransferEncodingChunkedParser::parse()
{
	std::string buffer = _parse_context.request.get_current_chunk();
	buffer.append(RequestStringUtils::consume_next_line(_parse_context.raw_bits));

	// std::cout << "RAW BITS: " << std::endl << _parse_context.raw_bits << std::endl;
	// std::cout << "TEMP: " << std::endl << temp << std::endl;
	// std::cout << "_parse_context.request.get_current_chunk_size(): " << _parse_context.request.get_current_chunk_size() << std::endl;
	// std::cout << "_parse_context.request.get_current_chunk_size_actual(): " << _parse_context.request.get_current_chunk_size_actual() << std::endl;

	while (!buffer.empty())
	{
		if (!_parse_context.request.get_current_chunk_size()) {
			if (!_tryGetNewChunk(buffer)) return ;
		}
		else {
			_chunk_size = _parse_context.request.get_current_chunk_size();
		}

		if (_isFinalChunk(buffer) || _isBad(buffer)) return ;
		if (_isComplete(buffer)) continue ;

		_parse_context.request.set_current_chunk(std::move(buffer));
		if (_parse_context.raw_bits.empty()) break;

		if (buffer.size() > _chunk_size) {
			std::cout << "[http-parser] Invalid chunk in transfer-encoding (too big)" << std::endl; 
			_parse_context.request.set_status_code(400);
		}
	}
}
