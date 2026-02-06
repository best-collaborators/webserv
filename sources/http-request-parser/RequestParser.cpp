#include "RequestParser.hpp"

RequestParser::RequestParser(Request &request, std::string &raw_bits)
: _request(request), _raw_bits(raw_bits) 
{
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
		_request.set_status_code(500);
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
	method = Trimmer::trim(method);
	percent_encoding(method);
	_request.set_header_value(key, method);
	return true;
}

std::string transform_to_lower(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
	return str;
}

void RequestParser::percent_encoding(std::string &buffer)
{
	auto pos = std::find(buffer.begin(), buffer.end(), '%');
	while (pos != buffer.end() && (pos + 1) != buffer.end() && (pos + 2) != buffer.end())
	{
		size_t index = pos - buffer.begin();
		char hex[3];
		hex[0] = buffer[index + 1];
		hex[1] = buffer[index + 2];
		hex[2] = '\0';

		try
		{
			char char_encoded = std::stoi(hex, nullptr, 16);
			buffer.replace(index, 3, 1, char_encoded);
		}
		catch(const std::exception& e) { std::cout << "[http-parser] Not a percent encoding character" << std::endl; }
		pos = std::find(buffer.begin() + index + 1, buffer.end(), '%');
	}
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
	
	if (_request.get_header_count(name)) {
		if (name == "host" || name == "content-length") {
			std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
			return false;
		}
		else {
			if (_request.get_header_value(name) != _buffer || _request.get_header_value(name) != _buffer)
			_request.append_header_value(name, _buffer);
			return true;
		}
	}

	_request.set_header_value(name, _buffer);
	return true;
}

int RequestParser::content_length_validation(){
	if (_request.get_header_count("forward-encoding")) {
		// std::cerr << "ERR: FORWARD-ENCODING + CONTENT LENGTH" << std::endl;
		std::cerr << "400 Bad Request forward-encoding + content-length" << std::endl; return 400;
	}
	try {
		size_t pos;
		const std::string content_length_str = _request.get_header_value("content-length");
		int test_length = std::stoll(content_length_str, &pos, 10);
		if (content_length_str.length() != pos) {
			// std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
			std::cerr << "400 Bad Request - content-length is NAN" << std::endl; return 400;
		}

		// max size is 1mb = 1048576b
		//! REQUEST TOO LARGE - REMOVE
		if (test_length < 0) {
			std::cerr << "413 Request Entity Too Large" << std::endl; return 413;
		}

	}
	catch(const std::exception& e) {
		// std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
		std::cerr << "400 Bad Request - content-length is NAN" << std::endl; return 400;
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
	_buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);

	if (_buffer == "") return 400;
	if (is_valid_request_line() == false){
		std::cerr << "400 Bad Request - request line is invalid" << std::endl;
		return 400;
	}

	if (_request.get_header_value("version") != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return 505;
	}

	if (_request.get_header_value("request-target").length() > 4096){
		std::cerr << "414 URI Too Long" << std::endl;
		return 414;
	}

	if (_request.get_header_value("method") != "GET"
		&& _request.get_header_value("method") != "POST"
		&& _request.get_header_value("method") != "OPTIONS"
		&& _request.get_header_value("method") != "DELETE") {
		std::cerr << "405 Not Allowed" << std::endl;
		return 405;
	}

	return 0;
}

uint RequestParser::validate_request_headers()
{
	_buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);
	while (!_buffer.empty()) {
		
		if (!is_valid_header()) {
			std::cerr << "400 Bad Request - header is invalid" << std::endl; return 400;
		}

		if (_request.amount_of_headers() >= 256) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl; return 400;
		}

		if (_request.get_header_count("content-length")) {
			if (int status_code = content_length_validation()) {
				return status_code;
			}
		}
		_buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);
	}

	if (_request.get_header_value("method") == "POST" && !_request.get_header_count("content-length")) {
		std::cerr << "411 Length Required" << std::endl; return 411;
	}

	return 0;
}

uint RequestParser::validate_request_body()
{
	//if post and content length is not 0 - error!
	if (_raw_bits.size() < 3) {
		if (_request.get_header_value("method") == "POST" && _request.get_header_count("content-length")) {
			std::cerr << "Body required!" << std::endl;
			return 400;
		}
		return 200;
	}

	return 0;
}

void RequestParser::parse_headers()
{
	if (_raw_bits.empty())
		RequestGenerator::create_post_request(_raw_bits);

	uint request_line_validation_status = validate_request_line();
	if (request_line_validation_status) {
		_request.set_status_code(request_line_validation_status);
		return ;
	}

	uint headers_validation_status = validate_request_headers();
	if (headers_validation_status) {
		_request.set_status_code(headers_validation_status);
		return ;
	}

	_request.set_status_code(200);
}

void RequestParser::parse_body()
{
	const std::string method = _request.get_header_value("method");
	const std::string content_type = _request.get_header_value("content-type");
	const std::string request_target = _request.get_header_value("request-target");

	if (method == "POST" && content_type.find("multipart/form-data") != std::string::npos) {

		std::string content_type = _request.get_header_value("_content_type");
		MultipartDataValidator validator(_multipartFormDatas, content_type, _buffer, _raw_bits);
		_request.set_status_code(validator.parse_multipart_data_form());
		if (_request.get_status_code() == 201) _uploaded_files_count++;
			return ;
	}
	if (method == "POST") {

		std::string upload_dir = "data/";
		std::string filename = _request.get_header_value("x-filename");

		if (filename.empty()) {
			if (content_type.empty())
				filename = std::to_string(_uploaded_files_count % 3) + "-updoad.bin";
			else 
				filename = std::to_string(_uploaded_files_count % 3) + "-updoad" + HttpContentType::get_extension_by_content_type(content_type);
		}

		std::fstream fout(upload_dir + filename, std::ios::binary | std::ios::out);
		if (!fout) {
			std::cerr << "[http] Error happend while writing into " << filename << std::endl;
			_request.set_status_code(500);
			return ;
		}

		fout.write(_raw_bits.c_str(), _raw_bits.size());

		_request.set_status_code(201);
		_uploaded_files_count++;
		fout.close();
		return ;
	}

	_request.set_status_code(200);
}

uint RequestParser::get_status_code()
{
	return _request.get_status_code();
}