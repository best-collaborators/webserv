#include "RequestParser.hpp"

RequestParser::RequestParser() {
	try 
	{
		for (const auto& entry : std::filesystem::directory_iterator("data")) {
			if (std::filesystem::is_regular_file(entry.status())) {
				++_uploaded_files_count;
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "[data] Cannot retrieve amount of uploaded files" << std::endl;
		_status_code = 500;
	}
}

std::string RequestParser::get_regex_value(std::string &line, std::regex regex_method)
{
	std::smatch matches;
	std::regex_search(line, matches, regex_method);
	if (matches.empty()) return "";
	std::string matched_string = matches.str(1);
	line = matches.suffix();
	return matched_string;
}


bool RequestParser::add_value_to_map(
	const char *regex_str,
	std::string errmsg,
	std::string key
)
{
	std::regex reg_method(regex_str);

	std::string method = get_regex_value(_buffer, reg_method);
	if (method == "") {
		std::cout << errmsg << std::endl;
		return false;
	}
	_headers[key] = Trimmer::trim(method);
	return true;
}

std::string transform_to_lower(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
	return str;
}

bool RequestParser::is_valid_header()
{
	std::regex reg_method(REGEX_HTTP_HEADER);
	std::string name = get_regex_value(_buffer, reg_method);
	if (name == "") return false;
	
	Trimmer::trim(_buffer);
	if (_buffer.length() > 8192) return false;

	Trimmer::trim(name);
	name.erase(name.length() - 1);
	transform_to_lower(name);

	if ((name == "host" || name == "content-length") && _buffer.empty())
		return false;
	
	if (_headers.count(name)) {
		if (name == "host" || name == "content-length") {
			std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
			return false;
		}
		else {
			if (_headers[name] != _buffer || _headers[name] != _buffer)
			_headers[name] = _headers[name] + "," + _buffer;
			return true;
		}
	}
	_headers[name] = _buffer;
	return true;
}

int RequestParser::content_length_validation(){
	if (_headers.count("forward-encoding")) {
		// std::cerr << "ERR: FORWARD-ENCODING + CONTENT LENGTH" << std::endl;
		std::cerr << "400 Bad Request" << std::endl; return 400;
	}
	try {
		size_t pos;
		int test_length = std::stoll(_headers["content-length"], &pos, 10);
		if (_headers["content-length"].length() != pos) {
			// std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
			std::cerr << "400 Bad Request" << std::endl; return 400;
		}

		// max size is 1mb = 1048576b
		//! REQUEST TOO LARGE - REMOVE
		if (test_length < 0) {
			std::cerr << "413 Request Entity Too Large" << std::endl; return 413;
		}

	}
	catch(const std::exception& e) {
		// std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
		std::cerr << "400 Bad Request" << std::endl; return 400;
	}
	return 0;
}

bool RequestParser::is_valid_request_line()
{
	if (_buffer.length() > 8192) return false;
	return (add_value_to_map(REGEX_HTTP_METHOD, ERROR_HTTP_METHOD, "method")
	&& add_value_to_map(REGEX_HTTP_REQUEST_TARGET, ERROR_HTTP_REQUEST_TARGET, "request-target")
	&& add_value_to_map(REGEX_HTTP_VESRION, ERROR_HTTP_VESRION, "version"));
}

uint RequestParser::validate_request_line()
{
	_buffer = ValidatorHelpers::cut_after_new_line(_request);

	if (_buffer == "") return 400;
	if (is_valid_request_line() == false){
		std::cerr << "400 Bad Request" << std::endl;
		return 400;
	}

	if (_headers["version"] != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return 505;
	}

	if (_headers["request-target"].length() > 4096){
		std::cerr << "414 URI Too Long" << std::endl;
		return 414;
	}

	if (_headers["method"] != "GET"
		&& _headers["method"] != "POST"
		&& _headers["method"] != "OPTIONS"
		&& _headers["method"] != "DELETE") {
		std::cerr << "405 Not Allowed" << std::endl;
		return 405;
	}

	return 0;
}

uint RequestParser::validate_request_headers()
{
	_buffer = ValidatorHelpers::cut_after_new_line(_request);
	while (!_buffer.empty()) {
		
		if (!is_valid_header()) {
			std::cerr << "400 Bad Request" << std::endl; return 400;
		}

		if (_headers.size() >= 256) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl; return 400;
		}

		if (_headers.count("content-length")) {
			if (int status_code = content_length_validation()) {
				return status_code;
			}
		}
		_buffer = ValidatorHelpers::cut_after_new_line(_request);
	}

	if (_headers["method"] == "POST" && !_headers.count("content-length")) {
		std::cerr << "411 Length Required" << std::endl; return 411;
	}

	return 0;
}

uint RequestParser::validate_request_body()
{
	//if post and content length is not 0 - error!
	if (_request.size() < 3) {
		if (_headers["method"] == "POST" && _headers.count("content-length")) {
			std::cerr << "Body required!" << std::endl;
			return 400;
		}
		return 200;
	}

	return 0;
}

void RequestParser::parse_headers(std::string request)
{
	_headers.clear();
	_request = request;

	if (_request.empty())
		RequestGenerator::create_post_request(_request);

	uint request_line_validation_status = validate_request_line();
	if (request_line_validation_status) _status_code = request_line_validation_status;

	uint headers_validation_status = validate_request_headers();
	if (headers_validation_status) _status_code = headers_validation_status;

	_status_code = 200;
}

void RequestParser::parse_body(std::string request)
{
	_request = request;
	if (_headers["method"] == "POST" && _headers["content-type"].find("multipart/form-data") != std::string::npos) {
		MultipartDataValidator validator(_multipartFormDatas, _headers, _buffer, _request);
		_status_code = validator.parse_multipart_data_form();
		if (_status_code == 201) _uploaded_files_count++;
		return ;
	}

	if (_headers["method"] == "POST") {
		std::string upload_dir = "data/";

		std::string filename = _headers["x-filename"];
		if (filename.empty()) {
			std::string content_type = _headers["content-type"];
			if (content_type.empty())
				filename = std::to_string(_uploaded_files_count % 3) + "-updoad.bin";
			else 
				filename = std::to_string(_uploaded_files_count % 3) + "-updoad" + HttpContentType::get_extension_by_content_type(content_type);
		}

		std::fstream fout(upload_dir + filename, std::ios::binary | std::ios::out);
		if (!fout) {
			std::cerr << "[http] Error happend while writing into " << filename << std::endl;
			_status_code = 500;
			return ;
		}

		fout.write(_request.c_str(), _request.size());

		_status_code = 201;
		_uploaded_files_count++;
		fout.close();
		return ;
	}

	_status_code = 200;
}

HttpRequest RequestParser::create_request_parse_result()
{
	return {
		_status_code,
		std::move(_headers)
	};
}

uint RequestParser::get_status_code()
{
	return _status_code;
}