#ifndef HTTP_BODY_PARSER
#define HTTP_BODY_PARSER

#include "MultipartDataParser.hpp"
#include "TransferEncodingChunkedParser.hpp"
#include "FileUploadHandler.hpp"

#include "ParseContext.hpp"
#include "IParser.hpp"

class HttpBodyParser : public IParser
{
private:
	ParseContext					&_parse_context;
	std::vector<MultipartFormData>	_multipartFormDatas;

	void _handleMultipart();
	void _handleChunked();
	void _handleRawUpload();

	HttpBodyParser() = delete;
	HttpBodyParser(const HttpBodyParser && other) = delete;
	HttpBodyParser(const HttpBodyParser & other) = delete;

public:
	HttpBodyParser( ParseContext &parse_context );
	~HttpBodyParser() = default;

	void parse();
};

#endif /* HTTP_BODY_PARSER */
