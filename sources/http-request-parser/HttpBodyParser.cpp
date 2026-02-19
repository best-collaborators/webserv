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
	FileUploadHandler file_uploader("data/", _parse_context.request);

	TransferEncodingChunkedParser chunked_parser(_parse_context);
	chunked_parser.parse();

	// std::cout << std::boolalpha << _parse_context.request.chunkHandler().isReceived() << std::endl;
	if (!_parse_context.request.chunkHandler().isReceived()) return ;

	file_uploader.write_into_file(_parse_context.request.get_body());
	return ;
}

void HttpBodyParser::_handleRawUpload()
{
	FileUploadHandler file_uploader("data/", _parse_context.request);
	file_uploader.write_into_file(_parse_context.raw_bits);
	_parse_context.request.set_status_code(HttpStatus::e_code::NO_CONTENT);
}

void HttpBodyParser::parse()
{
	RequestType body_status = _parse_context.request.getBodyStatus();

	std::cout << body_status << std::endl;
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
