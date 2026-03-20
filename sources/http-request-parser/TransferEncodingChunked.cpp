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

void TransferEncodingChunkedParser::parse()
{
	Request &request = _parse_context.request;
	while (true)
	{
		_chunk_size = request.chunkHandler().getExpectedSize();
		if (_chunk_size > request.getFile().getMaxBodySize() ||
			request.get_body().size() > request.getFile().getMaxBodySize()) {
			request.set_status_code(HttpStatus::e_code::CONTENT_TOO_LARGE);
			request.chunkHandler().finalize();
			Log::warning("Body size is too large", "http-parser");
			return ;
		}
		if (_chunk_size == 0) {
			auto pos = _parse_context.raw_bits.find("\r\n");
			if (pos == std::string::npos) {
				break;
			}

			std::string line = _parse_context.raw_bits.substr(0, pos);
			_parse_context.raw_bits.erase(0, pos + 2);
			if (!_tryGetNewChunk(line)) return ;

			if (_chunk_size == 0) {
				request.set_status_code(HttpStatus::e_code::OK);
				request.chunkHandler().finalize();
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
			request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			request.chunkHandler().finalize();
			return ;
		}
		request.get_body().append(buffer);
		_parse_context.raw_bits.erase(0, 2);
		request.chunkHandler().reset();
		_chunk_size = 0;
	}
}
