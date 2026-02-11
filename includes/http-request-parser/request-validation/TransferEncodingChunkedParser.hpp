#ifndef TRANSFER_ENCODING_CHUNKED_PARSER
#define TRANSFER_ENCODING_CHUNKED_PARSER

#include "Request.hpp"
#include "RequestStringUtils.hpp"
#include "RegexMatcher.hpp"
#include "HttpRegexPatterns.hpp"

class TransferEncodingChunkedParser
{
private:
	Request							&_request;
	std::string						&_raw_bits;
	std::string						_buffer;

	TransferEncodingChunkedParser() = delete;
	TransferEncodingChunkedParser(const TransferEncodingChunkedParser && other) = delete;
	TransferEncodingChunkedParser(const TransferEncodingChunkedParser & other) = delete;

public:
	TransferEncodingChunkedParser( std::string &_raw_bits, Request &_request );
	~TransferEncodingChunkedParser() = default;

	void parse();
};

#endif /* TRANSFER_ENCODING_CHUNKED_PARSER */
