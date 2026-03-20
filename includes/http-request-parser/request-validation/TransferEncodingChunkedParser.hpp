#ifndef TRANSFER_ENCODING_CHUNKED_PARSER
#define TRANSFER_ENCODING_CHUNKED_PARSER

#include "Request.hpp"
#include "RequestStringUtils.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"

#include "ParseContext.hpp"

#include "IParser.hpp"

class TransferEncodingChunkedParser : public IParser
{
private:
	ParseContext &_parse_context;
	size_t		_chunk_size = 0;
	static std::string old_raw_bits;

	TransferEncodingChunkedParser() = delete;
	TransferEncodingChunkedParser(const TransferEncodingChunkedParser && other) = delete;
	TransferEncodingChunkedParser(const TransferEncodingChunkedParser & other) = delete;

	bool _tryGetNewChunk( std::string &buffer);

public:
	TransferEncodingChunkedParser( ParseContext &parse_context );
	~TransferEncodingChunkedParser() = default;

	void parse();
};

#endif /* TRANSFER_ENCODING_CHUNKED_PARSER */
