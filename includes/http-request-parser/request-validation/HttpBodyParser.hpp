#ifndef HTTP_BODY_PARSER
#define HTTP_BODY_PARSER

#include "Request.hpp"
#include "MultipartDataValidator.hpp"
#include "TransferEncodingChunkedParser.hpp"
#include "FileUploadHandler.hpp"

class HttpBodyParser
{
private:
	Request							&_request;
	std::string						&_raw_bits;
	std::string						_buffer;
	std::vector<MultipartFormData>	_multipartFormDatas;

	HttpBodyParser() = delete;
	HttpBodyParser(const HttpBodyParser && other) = delete;
	HttpBodyParser(const HttpBodyParser & other) = delete;

public:
	HttpBodyParser( std::string &_raw_bits, Request &_request );
	~HttpBodyParser() = default;

	uint validate_request_body();
	void parse();
};

#endif /* HTTP_BODY_PARSER */
