#include "MultipartDataParser.hpp"

MultipartDataParser::MultipartDataParser(
			std::vector <MultipartFormData> &multipartFormDatas,
			ParseContext &parse_context
		) : _multipartFormDatas(multipartFormDatas), _parse_context(parse_context) { }

std::string MultipartDataParser::_getMultipartFormBoundary()
{
	std::string boundary;
	std::smatch match;

	std::string content_type = _parse_context.request.getContentType();
	if (!std::regex_search(content_type, match, HttpRegexPatterns::BOUNDARY())) {
		return "";
	}

	boundary = match[1];
	Trimmer::trim(boundary);
	Trimmer::trim(boundary, '\"');
	if (boundary.size() > 70) {
		return "";
	}

	std::regex reg1(R"(^\s*$)");
	if (std::regex_match(boundary, reg1)) {
		return "";
	}
	return boundary;
}

bool MultipartDataParser::_checkMultipartHeader(const std::string &buffer, MultipartFormData &multipart_form_data)
{
	std::smatch m;
	if (!std::regex_search(buffer, m, HttpRegexPatterns::CONTENT_DISPOSITION())) { return false; }

	multipart_form_data.set_name(m[1]);
	multipart_form_data.set_filename(m[3]);
	return true;
}

bool MultipartDataParser::_checkMultipartContentType(const std::string &buffer, MultipartFormData &multipart_form_data)
{	
	if (buffer.empty())
	{
		multipart_form_data.set_content_type("text/plain");
		return true;
	}

	std::regex reg(HttpRegexPatterns::CONTENT_TYPE());
	std::smatch m;
	if (!std::regex_search(buffer, m, reg)) { return false; }

	multipart_form_data.set_content_type(m[1]);
	return true;
}

//TODO: CHANGE IT TO FILEUPLOAD CLASS
void MultipartDataParser::_createMultipartDataFormFiles()
{
	std::string upload_dir = "data/";
	for (auto &data : _multipartFormDatas)
	{
		// data.print_all_data();
		if (!data.get_filename().empty())
		{
			FileUploadHandler fileuploader(upload_dir, _parse_context.request);
			fileuploader.write_into_file(data.get_content());
		}
		data.clear();
	}
}

HttpStatus::e_code MultipartDataParser::_parseMultipartFormData(std::string &buffer, MultipartFormData &multipart_form_data)
{
	if (!_checkMultipartHeader(buffer, multipart_form_data)) {
		std::cerr << "400 Bad Request - bad multipart header" << std::endl;
		return HttpStatus::e_code::BAD_REQUEST;
	}

	buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	if (!_checkMultipartContentType(buffer, multipart_form_data)) {
		std::cerr << "400 Bad Request - bad multipart content type" << std::endl;
		return HttpStatus::e_code::BAD_REQUEST;
	}

	if (!multipart_form_data.get_content_type().empty())
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);

	return HttpStatus::e_code::OK;
}

HttpStatus::e_code MultipartDataParser::truncate_boundary(
	std::string &buffer,
	BoundaryContext &boundary_context,
	MultipartFormData &multipart_form_data
)
{
	std::size_t boundary_pos = _parse_context.raw_bits.find(boundary_context.boundary_marker);
	std::size_t boundary_pos_end = boundary_pos;

	if (boundary_pos == std::string::npos) {

		boundary_pos = _parse_context.raw_bits.find(boundary_context.closing_boundary_marker);
		boundary_pos_end = boundary_pos + boundary_context.closing_boundary_marker.size();
		
		boundary_context.is_end = true;

		if (boundary_pos == std::string::npos) {
			std::cerr << "400 Bad Request - no end boundary" << std::endl;
			return HttpStatus::e_code::BAD_REQUEST;
		}
	}

	buffer = _parse_context.raw_bits.substr(0, boundary_pos);
	multipart_form_data.set_content(buffer);

	_parse_context.raw_bits.erase(0, boundary_pos_end);
	_multipartFormDatas.push_back(multipart_form_data);

	return HttpStatus::e_code::OK;
}

bool MultipartDataParser::_isValidMultipartForm()
{
	std::string body;
	_boundary_context.boundary_marker = "--" + _boundary_context.boundary + "\r\n";
	_boundary_context.closing_boundary_marker =  "--" + _boundary_context.boundary + "--" + "\r\n";

	std::string buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	while (_parse_context.raw_bits.size() && !_boundary_context.is_end)
	{
		if (std::memcmp(_parse_context.raw_bits.data(), _boundary_context.boundary_marker.data(), _boundary_context.boundary_marker.size()) != 0) {
			std::cerr << "[HTTP-PARSER/MULTIPART] Wrong boundary." << std::endl; 
			_parse_context.request.set_status_code(HttpStatus::e_code::NOT_FOUND);
			return false;
		}
		_parse_context.raw_bits.erase(0, _boundary_context.boundary_marker.size());

		MultipartFormData multipart_form_data("", "", "");
		HttpStatus::e_code parse_multipart_form_data_status = _parseMultipartFormData(buffer, multipart_form_data);
		if (HttpStatus::is_bad(parse_multipart_form_data_status)) {
			std::cerr << "[HTTP-PARSER/MULTIPART] Multipart format is invalid." << std::endl;
			_parse_context.request.set_status_code(parse_multipart_form_data_status);
			return false;
		}

		HttpStatus::e_code trancate_boundary_status = truncate_boundary(buffer, _boundary_context, multipart_form_data);
		if (HttpStatus::is_bad(trancate_boundary_status)) {
			std::cerr << "[HTTP-PARSER/MULTIPART] Error occured with boundary extraction." << std::endl;
			_parse_context.request.set_status_code(trancate_boundary_status);
			return false;
		}

		multipart_form_data.print_all_data();
	}
	return true;
}

bool MultipartDataParser::_isBoundaryEmpty()
{
	_boundary_context.boundary = _getMultipartFormBoundary();
	if (_boundary_context.boundary.empty()) {
		std::cerr << "400 Bad Request - boundary empty." << std::endl;
		_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
		return true;
	}
	return false;
}

void MultipartDataParser::parse()
{
	if (_isBoundaryEmpty()) return ;

	if (!_isValidMultipartForm()) return ;
	_createMultipartDataFormFiles();
}
