#ifndef TRANSFER_ENCODING_CHUNKED_PARSER
#define TRANSFER_ENCODING_CHUNKED_PARSER

#include "Request.hpp"
#include "RequestStringUtils.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"

#include "ParseContext.hpp"

class TransferEncodingChunkedParser
{
private:
	ParseContext &_parse_context;
	unsigned long long				_chunk_size = 0;

	TransferEncodingChunkedParser() = delete;
	TransferEncodingChunkedParser(const TransferEncodingChunkedParser && other) = delete;
	TransferEncodingChunkedParser(const TransferEncodingChunkedParser & other) = delete;

	bool _isFinalChunk( std::string &buffer);
	bool _tryGetNewChunk( std::string &buffer);
	bool _isBad( std::string &buffer);
	bool _isComplete( std::string &buffer);

public:
	TransferEncodingChunkedParser( ParseContext &parse_context );
	~TransferEncodingChunkedParser() = default;

	void parse();
};

#endif /* TRANSFER_ENCODING_CHUNKED_PARSER */
