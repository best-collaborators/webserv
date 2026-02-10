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
	if (_request.get_header_count("transfer-encoding")) {
		// std::cerr << "ERR: TRANSFER-ENCODING + CONTENT LENGTH" << std::endl;
		std::cerr << "400 Bad Request transfer-encoding + content-length" << std::endl; return 400;
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

	if (_request.get_header_value("method") == "POST"
		&& !_request.get_header_count("content-length")
		&& !_request.get_header_count("transfer-encoding")) {

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

void RequestParser::parse_chunked_encoding()
{
	std::string buffer = _request.get_current_chunk();

	std::string temp = ValidatorHelpers::cut_after_new_line(_raw_bits);
	if (temp.empty() && !_raw_bits.empty()) {
		temp += std::move(_raw_bits);
		_raw_bits = "";
	}
	buffer += temp;

	// std::cout << "RAW BITS: " << std::endl << _raw_bits << std::endl;
	// std::cout << "TEMP: " << std::endl << temp << std::endl;
	std::cout << "BUFFER: " << std::endl << buffer << std::endl;
	std::cout << "_request.get_current_chunk_size(): " << _request.get_current_chunk_size() << std::endl;
	std::cout << "_request.get_current_chunk_size_actual(): " << _request.get_current_chunk_size_actual() << std::endl;

	while (!buffer.empty())
	{
		unsigned long long chunk_size = 0;
		if (!_request.get_current_chunk_size()) {
			std::regex reg("^([0-9a-f]+)$");
			std::string hex = get_regex_value(buffer, reg);
			try {
				chunk_size = std::stoull(hex, nullptr, 16);
			}
			catch(const std::exception& e) { 
				std::cout << "[http-parser] Invalid size in transfer-encoding --> " << hex << std::endl; 
				_request.set_status_code(400);
				_request.set_is_chunk_received(true);
				_request.set_current_chunk("");
				_request.set_chunk_size(0);
				return ;
			}
			_request.set_chunk_size(chunk_size);

			buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);
			if (buffer.empty() && !_raw_bits.empty()) {
				buffer = std::move(_raw_bits);
				_raw_bits = "";
			}
		}
		else {
			chunk_size = _request.get_current_chunk_size();
		}

		if (chunk_size == 0 && buffer.empty() && _raw_bits.empty()) {
			_request.set_status_code(200);
			_request.set_is_chunk_received(true);
			return ;
		}

		if ((chunk_size > 0 && buffer.empty())) {
			std::cout << "[http-parser] Invalid chunk in transfer-encoding" << std::endl; 
			_request.set_status_code(400);
			return ;
		}

		if (buffer.size() == chunk_size) {
			_request.append_body_value(buffer);
			_request.set_current_chunk("");
			_request.set_chunk_size(0);
			buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);
			continue ;
		}

		while (buffer.size() >= chunk_size && !_raw_bits.empty()) {
			std::cout << "buffer size: " << buffer.size() << std::endl;
			std::string temp = ValidatorHelpers::cut_after_new_line(_raw_bits);
			if (temp.empty() && !_raw_bits.empty()) {
				temp += std::move(_raw_bits);
				_raw_bits = "";
			}
			buffer += temp;
		}
		if (buffer.size() >= chunk_size)
			_request.append_body_value(buffer);
		_request.set_current_chunk(std::move(buffer));
		if (_raw_bits.empty()) break;
	}
}

void RequestParser::write_into_file(std::string upload_dir, std::string filename, std::string _raw_bits, std::string content_type)
{
	if (_request.get_status_code() > 300)
		return ;

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

	_uploaded_files_count++;
	fout.close();
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

	if (method == "POST" && _request.get_header_count("transfer-encoding") > 0) {

		parse_chunked_encoding();

		if (!_request.is_chunk_received()) return ;

		write_into_file("data/", _request.get_header_value("x-filename"), _request.get_body(), content_type);
		return ;
	}

	if (method == "POST") {
		write_into_file("data/", _request.get_header_value("x-filename"), _raw_bits, content_type);
		_request.set_status_code(204);
		return ;
	}

	_request.set_status_code(200);
}

uint RequestParser::get_status_code()
{
	return _request.get_status_code();
}