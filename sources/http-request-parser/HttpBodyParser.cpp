#include "HttpBodyParser.hpp"

HttpBodyParser::HttpBodyParser( ParseContext &parse_context )
: _parse_context(parse_context) { }

void HttpBodyParser::_handleMultipart()
{
	std::string content_type = _parse_context.request.getContentType();
	MultipartDataValidator validator(_multipartFormDatas, content_type, _parse_context.raw_bits);
	_parse_context.request.set_status_code(validator.parse_multipart_data_form());
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
	_parse_context.request.set_status_code(204);
}

void HttpBodyParser::parse()
{
	if (!_parse_context.request.expectsBody()) {
		return ;
	}

	if (_parse_context.request.isMultipart()) {
		_handleMultipart(); return ;
	}

	if (_parse_context.request.isChunked()) {
		_handleChunked(); return ;
	}

	_handleRawUpload();
}
