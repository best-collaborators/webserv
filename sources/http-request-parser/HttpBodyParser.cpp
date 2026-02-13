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

	if (!_parse_context.request.is_chunk_received()) return ;

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
	RequestBodyStatus body_status = _parse_context.request.getBodyStatus();

	switch (body_status)
	{
	case RequestBodyStatus::CGI:
		/* code */
		break;

	case RequestBodyStatus::CHUNKED:
		_handleChunked();
		break;

	case RequestBodyStatus::MULTIPART:
		_handleMultipart();
		break;

	case RequestBodyStatus::RAW_BODY:
		_handleRawUpload();
		break;

	default:
		break;
	}
	
}
