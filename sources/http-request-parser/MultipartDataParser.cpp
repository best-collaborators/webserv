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
	std::regex reg(HttpRegexPatterns::CONTENT_TYPE());
	std::smatch m;
	if (!std::regex_search(buffer, m, reg)) { return false; }

	multipart_form_data.set_content_type(m[1]);
	return true;
}

void MultipartDataParser::_createMultipartDataFormFiles()
{
	for (auto &data : _multipartFormDatas)
	{
		if (!data.get_filename().empty())
		{
			data.print_all_data();

			FileUploadHandler file_uploader(
				_parse_context.request.getFile().getFullFilename(),
				_parse_context.request,
				data.get_filename());
			file_uploader.write_into_file(data.get_content());
		}
		data.clear();
	}
}

HttpStatus::e_code MultipartDataParser::_parseMultipartFormData(std::string &buffer, MultipartFormData &multipart_form_data)
{
	if (!_checkMultipartHeader(buffer, multipart_form_data)) {
		Log::debug("Bad multipart header");
		return HttpStatus::e_code::BAD_REQUEST;
	}

	buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	if (buffer.empty())
	{
		multipart_form_data.set_content_type("text/plain");
		return HttpStatus::e_code::OK;
	}

	if (!_checkMultipartContentType(buffer, multipart_form_data)) {
		Log::debug("Bad multipart content type");
		return HttpStatus::e_code::BAD_REQUEST;
	}
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
	std::size_t boundary_pos_end = boundary_pos + boundary_context.boundary_marker.size();

	if (boundary_pos == std::string::npos) {
		Log::debug("No end boundary");
		return HttpStatus::e_code::BAD_REQUEST;
	}

	if ((boundary_context.closing_boundary_marker.size() + 2) == _parse_context.raw_bits.size() &&
		_parse_context.raw_bits.compare(0, boundary_context.closing_boundary_marker.size(),
			boundary_context.closing_boundary_marker) == 0)
		boundary_context.is_end = true;

	buffer.append(_parse_context.raw_bits.substr(0, boundary_pos));
	multipart_form_data.set_content(buffer);
	_multipartFormDatas.push_back(multipart_form_data);

	buffer = _parse_context.raw_bits.substr(boundary_pos, boundary_pos + boundary_context.boundary_marker.size());
	_parse_context.raw_bits.erase(0, boundary_pos_end + 2);

	if (boundary_context.is_end && _parse_context.raw_bits != "\r\n") {
		Log::debug("No end boundary");
		return HttpStatus::e_code::BAD_REQUEST;
	}
	return HttpStatus::e_code::OK;
}

bool MultipartDataParser::_isValidMultipartForm()
{
	std::string body;
	_boundary_context.boundary_marker = "--" + _boundary_context.boundary;
	_boundary_context.closing_boundary_marker =  "--" + _boundary_context.boundary + "--";

	std::string buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
	while (_parse_context.raw_bits.size() && !_boundary_context.is_end)
	{
		if (_boundary_context.boundary_marker != buffer && _boundary_context.closing_boundary_marker != buffer) {
			Log::debug("Wrong boundary");
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			return false;
		}
		buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
		if (buffer.empty()) {
			buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
		}
		else if (_parse_context.raw_bits.empty()) {
			Log::debug("Multipart format is invalid");
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			return false;
		}
		MultipartFormData multipart_form_data("", "", "");

		HttpStatus::e_code parse_multipart_form_data_status = _parseMultipartFormData(buffer, multipart_form_data);
		if (HttpStatus::is_bad(parse_multipart_form_data_status)) {
			Log::debug("Multipart format is invalid");
			_parse_context.request.set_status_code(parse_multipart_form_data_status);
			return false;
		}
		if (buffer.empty()) {
			buffer = RequestStringUtils::cut_after_new_line(_parse_context.raw_bits);
		}
		else if (_parse_context.raw_bits.empty()) {
			Log::debug("Multipart format is invalid");
			_parse_context.request.set_status_code(HttpStatus::e_code::BAD_REQUEST);
			return false;
		}

		HttpStatus::e_code truncated_boundary_status = truncate_boundary(buffer, _boundary_context, multipart_form_data);
		if (HttpStatus::is_bad(truncated_boundary_status)) {
			Log::debug("Error occurred with boundary extraction");
			_parse_context.request.set_status_code(truncated_boundary_status);
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
		Log::debug("Boundary empty");
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
