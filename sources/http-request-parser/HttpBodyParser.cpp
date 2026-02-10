#include "HttpBodyParser.hpp"

HttpBodyParser::HttpBodyParser( std::string &raw_bits, Request &request )
: _request(request), _raw_bits(raw_bits) { }

uint HttpBodyParser::validate_request_body()
{
	//if post and content length is not 0 - error!
	if (_raw_bits.size() < http::limits::min_body_length) {
		if (_request.get_header_value("method") == "POST" && _request.get_header_count("content-length")) {
			std::cerr << "Body required!" << std::endl;
			return 400;
		}
		return 200;
	}

	return 0;
}

void HttpBodyParser::parse()
{
	const std::string method = _request.get_header_value("method");
	const std::string content_type = _request.get_header_value("content-type");
	const std::string request_target = _request.get_header_value("request-target");

	if (method == "POST" && content_type.find("multipart/form-data") != std::string::npos) {

		std::string content_type = _request.get_header_value("_content_type");
		MultipartDataValidator validator(_multipartFormDatas, content_type, _buffer, _raw_bits);
		_request.set_status_code(validator.parse_multipart_data_form());
	}

	FileUploadHandler file_uploader("data/", _request);
	if (method == "POST" && _request.get_header_count("transfer-encoding") > 0) {

		TransferEncodingChunkedParser chunked_parser(_raw_bits, _request);
		chunked_parser.parse();

		if (!_request.is_chunk_received()) return ;

		file_uploader.write_into_file(_request.get_body());
		return ;
	}

	if (method == "POST") {
		file_uploader.write_into_file(_raw_bits);
		_request.set_status_code(204);
		return ;
	}

	_request.set_status_code(200);
}
