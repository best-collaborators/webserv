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
		Log::error("Invalid size in transfer-encoding", "http-parser");
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
		Log::error("Body is too big in transfer-encoding _parse_context.raw_bits.size()", "http-parser");
		_parse_context.request.chunkHandler().finalize();
		return true;
	}

	if (_parse_context.request.get_body().size() > _parse_context.request.getFile().getMaxBodySize()) {
		_parse_context.request.set_status_code(HttpStatus::e_code::CONTENT_TOO_LARGE);
		Log::error("Body is too big in transfer-encoding", "http-parser");
		_parse_context.request.chunkHandler().finalize();
		return true;
	}

	if (_chunk_size == 0 && buffer.empty()) {
		_parse_context.request.set_status_code(HttpStatus::e_code::OK);
		_parse_context.request.chunkHandler().finalize();
		Log::debug("Received final chunk", "http-parser");
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isBad( std::string &buffer )
{
	if (_chunk_size > 0 && buffer.empty()) {
		Log::error("Invalid chunk in transfer-encoding empty buffer", "http-parser");
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return true;
	}
	return false;
}

bool TransferEncodingChunkedParser::_isComplete( std::string &buffer )
{
	if (buffer.size() == _chunk_size) {
		_parse_context.request.get_body().append(buffer);
		_parse_context.request.chunkHandler().reset();
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
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
		Log::error("Invalid chunk in transfer-encoding no \\r\\n", "http-parser");
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		_parse_context.request.chunkHandler().finalize();
		return false;
	}
	buffer.append(_parse_context.raw_bits.substr(0, pos));
	_parse_context.raw_bits.erase(0, pos);
	return true;
}

void TransferEncodingChunkedParser::parse()
{
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
				Log::debug("Received final chunk", "http-parser");
				return ;
			}
		}

		if (_parse_context.raw_bits.size() < _chunk_size + 2)
			break;


		std::string buffer = _parse_context.raw_bits.substr(0, _chunk_size);
		_parse_context.raw_bits.erase(0, _chunk_size);

		if (_parse_context.raw_bits.substr(0, 2) != "\r\n")
		{
			Log::error("Invalid chunk in transfer-encoding no \\r\\n", "http-parser");
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			_parse_context.request.chunkHandler().finalize();
			return ;
		}
		_parse_context.request.get_body().append(buffer);
		_parse_context.raw_bits.erase(0, 2);
		_parse_context.request.chunkHandler().reset();
		_chunk_size = 0;
	}
}
