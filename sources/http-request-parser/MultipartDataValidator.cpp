#include "MultipartDataValidator.hpp"

MultipartDataValidator::MultipartDataValidator(
			std::vector <MultipartFormData> &multipartFormDatas,
			std::string &content_type,
			std::string &buffer,
			std::string &request
		) : _multipartFormDatas(multipartFormDatas), _content_type(content_type), _buffer(buffer), _request(request) { }

MultipartDataValidator::~MultipartDataValidator(){ }

std::string MultipartDataValidator::get_multipart_form_boundary()
{
	std::string boundary;
	std::smatch match;

	if (!std::regex_search(_content_type, match, HttpRegexPatterns::BOUNDARY())) {
		return "";
	}

	boundary = match[1];
	Trimmer::trim(boundary);
	Trimmer::trim(boundary, '\"');
	if (boundary.size() > 70) {
		return "";
	}

	//Check with wrong multiformdata request
	std::regex reg1("$\\s^");
	std::regex_search(boundary, match, reg1);
	if (!match.empty()) {
		return "";
	}
	return boundary;
}

bool MultipartDataValidator::check_multipart_header(MultipartFormData &multipart_form_data)
{
	std::smatch m;
	if (!std::regex_search(_buffer, m, HttpRegexPatterns::CONTENT_DISPOSITION())) { return false; }

	multipart_form_data.set_name(m[1]);
	multipart_form_data.set_filename(m[3]);
	return true;
}

bool MultipartDataValidator::check_multipart_content_type(MultipartFormData &multipart_form_data)
{	
	if (_buffer.empty())
	{
		multipart_form_data.set_content_type("text/plain");
		return true;
	}

	std::regex reg(HttpRegexPatterns::CONTENT_TYPE());
	std::smatch m;
	if (!std::regex_search(_buffer, m, reg)) { return false; }

	multipart_form_data.set_content_type(m[1]);
	return true;
}

//TODO: CHANGE IT TO FILEUPLOAD CLASS
uint MultipartDataValidator::create_multipart_data_form_files()
{
	std::string upload_dir = "data/";
	for (auto data : _multipartFormDatas)
	{
		// data.print_all_data();
		if (!data.get_filename().empty())
		{
			std::fstream fout(upload_dir + data.get_filename(), std::ios::binary | std::ios::out);
			if (!fout)
				return 500;
			fout.write(data.get_content().c_str(), data.get_content().size());
			fout.close();
		}
	}
	return 0;
}

uint MultipartDataValidator::parse_multipart_form_data(MultipartFormData &multipart_form_data)
{
	_buffer = RequestStringUtils::cut_after_new_line(_request);
	if (!check_multipart_header(multipart_form_data)) {
		std::cerr << "400 Bad Request - bad multipart header" << std::endl; return 400;
	}

	_buffer = RequestStringUtils::cut_after_new_line(_request);
	if (!check_multipart_content_type(multipart_form_data)) {
		std::cerr << "400 Bad Request - bad multipart content type" << std::endl; return 400;
	}

	if (!multipart_form_data.get_content_type().empty())
		_buffer = RequestStringUtils::cut_after_new_line(_request);

	return 0;
}

uint MultipartDataValidator::truncate_boundary(bool &is_end,
std::string &boundary_marker,
std::string &closing_boundary_marker,
MultipartFormData &multipart_form_data
)
{
	std::size_t boundary_pos = _request.find(boundary_marker);
	std::size_t boundary_pos_end = boundary_pos;

	if (boundary_pos == std::string::npos) {

		boundary_pos = _request.find(closing_boundary_marker);
		boundary_pos_end = boundary_pos + closing_boundary_marker.size();
		
		is_end = true;

		if (boundary_pos == std::string::npos) {
			std::cerr << "400 Bad Request - no end boundary" << std::endl; return 400;
		}
	}

	_buffer = _request.substr(0, boundary_pos);
	multipart_form_data.set_content(_buffer);

	_request.erase(0, boundary_pos_end);
	_multipartFormDatas.push_back(multipart_form_data);

	return 0;
}

uint MultipartDataValidator::parse_multipart_data_form()
{
	std::string boundary = get_multipart_form_boundary();
	if (boundary.empty()) {
		std::cerr << "400 Bad Request - boundary empty." << std::endl; return 400;
	}

	std::string body;
	std::string boundary_marker = "--" + boundary + "\r\n";
	std::string closing_boundary_marker =  "--" + boundary + "--" + "\r\n";

	bool is_end = false;
	while (_request.size() && !is_end)
	{
		if (std::memcmp(_request.data(), boundary_marker.data(), boundary_marker.size()) != 0) {
			std::cerr << "[HTTP-PARSER/MULTIPART] Wrong boundary." << std::endl; return 404;
		}
		_request.erase(0, boundary_marker.size());

		MultipartFormData multipart_form_data("", "", "");
		uint parse_multipart_form_data_status = parse_multipart_form_data(multipart_form_data);
		if (parse_multipart_form_data_status) {
			std::cerr << "[HTTP-PARSER/MULTIPART] Multipart format is invalid." << std::endl; return parse_multipart_form_data_status;
		}

		uint trancate_boundary_status = truncate_boundary(is_end, boundary_marker, closing_boundary_marker, multipart_form_data);
		if (trancate_boundary_status) {
			std::cerr << "[HTTP-PARSER/MULTIPART] Error occured with boundary extraction." << std::endl; return trancate_boundary_status;
		}

		multipart_form_data.print_all_data();
	}

	uint multipart_data_file_creation_status = create_multipart_data_form_files();
	if (multipart_data_file_creation_status) {
		std::cerr << "[HTTP-PARSER/MULTIPART] Error occured while creating files." << std::endl; return multipart_data_file_creation_status;
	}

	return 201;
}
