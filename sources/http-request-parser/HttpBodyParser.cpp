#include "HttpBodyParser.hpp"

HttpBodyParser::HttpBodyParser( ParseContext &parse_context )
: _parse_context(parse_context) { }

void HttpBodyParser::_handleMultipart()
{
	std::string content_type = _parse_context.request.getContentType();
	MultipartDataParser validator(_multipartFormDatas, _parse_context);
	validator.parse();
}

void HttpBodyParser::_handleChunked()
{
	FileUploadHandler file_uploader(_parse_context.request.getFile().getFullFilename(), _parse_context.request);

	TransferEncodingChunkedParser chunked_parser(_parse_context);
	chunked_parser.parse();

	if (!_parse_context.request.chunkHandler().isReceived()) return ;

	if (_parse_context.request.isCGI()) return ;

	file_uploader.write_into_file(_parse_context.request.get_body());
	return ;
}

void HttpBodyParser::_handleRawUpload()
{
	FileUploadHandler file_uploader(_parse_context.request.getFile().getFullFilename(), _parse_context.request);
	file_uploader.write_into_file(_parse_context.raw_bits);
}

void HttpBodyParser::parse()
{
	RequestType body_status = _parse_context.request.getBodyStatus();

	switch (body_status)
	{
	case RequestType::CHUNKED:
		_handleChunked();
		break;

	case RequestType::MULTIPART:
		_handleMultipart();
		break;

	case RequestType::RAW_BODY:
		_handleRawUpload();
		break;

	default:
		break;
	}
	
}
