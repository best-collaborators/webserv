#include "RequestParser.hpp"

RequestParser::RequestParser(std::unordered_map<std::string, std::string> &http_request_values)
: _http_request_values(http_request_values) { }

std::string RequestParser::get_regex_value(std::string &line, std::regex regex_method)
{
	std::smatch matches;
	std::regex_search(line, matches, regex_method);
	if (matches.empty()) return "";
	std::string matched_string = matches.str(1);
	line = matches.suffix();
	return matched_string;
}

bool RequestParser::is_valid_request_line()
{
	if (buffer.length() > 8192) return false;
	return (add_value_to_map(REGEX_HTTP_METHOD, ERROR_HTTP_METHOD, "method")
	&& add_value_to_map(REGEX_HTTP_REQUEST_TARGET, ERROR_HTTP_REQUEST_TARGET, "request-target")
	&& add_value_to_map(REGEX_HTTP_VESRION, ERROR_HTTP_VESRION, "version"));
}

bool RequestParser::add_value_to_map(
	const char *regex_str,
	std::string errmsg,
	std::string key
)
{
	std::regex reg_method(regex_str);
	std::string method = get_regex_value(buffer, reg_method);
	if (method == "") {
		std::cout << errmsg << std::endl;
		return false;
	}
	_http_request_values[key] = Trimmer::trim(method);
	return true;
}

bool RequestParser::is_valid_header()
{
	std::regex reg_method("(^\\S{1,256}:[ ]+)");
	std::string name = get_regex_value(buffer, reg_method);
	if (name == "") return false;
	
	Trimmer::trim(buffer);
	if (buffer.length() > 8192) return false;

	Trimmer::trim(name);
	name.erase(name.length() - 1);
	std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });

	if ((name == "host" || name == "content-length") && buffer.empty())
		return false;

	if (!_http_request_values[name].empty()) {
		if (name == "host" || name == "content-length") {
			std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
			return false;
		}
		else {
			if (_http_request_values[name] != buffer || _http_request_values[name] != buffer)
			_http_request_values[name] = _http_request_values[name] + "," + buffer;
			return true;
		}
	}
	_http_request_values[name] = buffer;
	return true;
}

int RequestParser::content_length_validation(){
	if (_http_request_values.count("forward-encoding")) {
		// std::cerr << "ERR: FORWARD-ENCODING + CONTENT LENGTH" << std::endl;
		std::cerr << "400 Bad Request" << std::endl; return 400;
	}
	try {
		size_t pos;
		int test_length = std::stoll(_http_request_values["content-length"], &pos, 10);
		if (_http_request_values["content-length"].length() != pos) {
			// std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
			std::cerr << "400 Bad Request" << std::endl; return 400;
		}

		// max size is 1mb = 1048576b
		if (test_length < 0 || test_length > 1048576) {
			std::cerr << "413 Request Entity Too Large" << std::endl; return 413;
		}

	}
	catch(const std::exception& e) {
		// std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
		std::cerr << "400 Bad Request" << std::endl; return 400;
	}
	return 0;
}

std::string RequestParser::get_multipart_form_boundary()
{
	std::string boundary;
	std::smatch match;

	std::regex reg("^multipart/form-data;\\s*boundary=([^;]+)");

	if (!std::regex_search(_http_request_values["content-type"], match, reg)) {
		return "";
	}

	boundary = match[1];
	Trimmer::trim(boundary);
	Trimmer::trim(boundary, '\"');
	if (boundary.size() > 70) {
		return "";
	}
	
	std::regex reg1("\\s");
	std::regex_search(boundary, match, reg1);
	if (!match.empty()) {
		return "";
	}
	return boundary;
}

bool RequestParser::check_multipart_header(MultipartFormData &multipart_form_data)
{
	std::regex reg("^[C,c]ontent-[D,d]isposition: form-data;\\s*name=\"(\\S{1,256})\";?\\s*(filename=\"(\\S{1,256})\")?");
	std::smatch m;
	if (!std::regex_search(buffer, m, reg)) { return false; }

	multipart_form_data.set_name(m[1]);
	multipart_form_data.set_filename(m[3]);
	return true;
}

bool RequestParser::check_multipart_content_type(MultipartFormData &multipart_form_data)
{	
	if (buffer.empty())
	{
		multipart_form_data.set_content_type("text/plain");
		return true;
	}

	std::regex reg("^[C,c]ontent-[T,t]ype:\\s*(\\S{1,256}\\/\\S{1,256})\\s*");
	std::smatch m;
	if (!std::regex_search(buffer, m, reg)) { return false; }

	multipart_form_data.set_content_type(m[1]);
	return true;
}

std::string cut_after_new_line(std::string &line)
{
	size_t pos = line.find("\r\n");
	if (pos == std::string::npos) return "";

	std::string temp_buffer = line.substr(0, pos);

	line.erase(0, pos + 2);
	return temp_buffer;
}

uint RequestParser::get_status_code(std::string request)
{
	buffer = "";

	if (request.empty())
		RequestGenerator::create_post_request(request);

	buffer = cut_after_new_line(request);

	if (buffer == "") return 400;
	if (is_valid_request_line() == false){
		std::cerr << "400 Bad Request" << std::endl;
		return 400;
	}

	if (_http_request_values["version"] != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		return 505;
	}

	if (_http_request_values["request-target"].length() > 4096){
		std::cerr << "414 URI Too Long" << std::endl;
		return 414;
	}

	if (_http_request_values["method"] != "GET"
		&& _http_request_values["method"] != "POST"
		&& _http_request_values["method"] != "DELETE") {
		std::cerr << "405 Not Allowed" << std::endl;
		return 405;
	}
		
	buffer = cut_after_new_line(request);
	while (!buffer.empty()) {
		
		if (!is_valid_header()) {
			std::cerr << "400 Bad Request" << std::endl; return 400;
		}

		if (_http_request_values.size() >= 256) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl; return 400;
		}

		if (_http_request_values.count("content-length")) {
			if (int status_code = content_length_validation()) {
				return status_code;
			}
		}
		buffer = cut_after_new_line(request);
	}

	if (_http_request_values["method"] == "POST" && !_http_request_values.count("content-length")) {
		std::cerr << "411 Length Required" << std::endl; return 411;
	}

	std::string boundary;
	if (_http_request_values["method"] == "POST" && _http_request_values["content-type"].find("multipart/form-data") != std::string::npos) {
		boundary = get_multipart_form_boundary();
		if (boundary.empty()) {
			std::cerr << "400 Bad Request" << std::endl; return 400;
		}
	}

	//if post and content length is not 0 - error!
	if (request.size() < 3) {
		if (_http_request_values["method"] == "POST" && _http_request_values.count("content-length")) {
			std::cerr << "Body required!" << std::endl;
			return 400;
		}
		return 200;
	}

	std::string body;

	std::string boundary_marker = "--" + boundary + "\r\n";
	std::string closing_boundary_marker =  "--" + boundary + "--" + "\r\n";

	while (request.size())
	{
		if (std::memcmp(request.data(), boundary_marker.data(), boundary_marker.size()) != 0)
		{
			std::cerr << "Wrong header" << std::endl;
			return 500;
		}

/* 		std::cout << "buffer |" << buffer << "|"  << "size: " << buffer.size() << std::endl;
		std::cout << "request |" << request << "|"  << "size: " << request.size() << std::endl; */

		request.erase(0, boundary_marker.size());

		MultipartFormData multipart_form_data;

		buffer = cut_after_new_line(request);
		if (!check_multipart_header(multipart_form_data)) {
			std::cerr << "400 Bad Request - bad multipart header" << std::endl; return 400;
		}

		buffer = cut_after_new_line(request);
		if (!check_multipart_content_type(multipart_form_data)) {
			std::cerr << "400 Bad Request - bad multipart content type" << std::endl; return 400;
		}

		if (!multipart_form_data.get_content_type().empty())
			buffer = cut_after_new_line(request);

		std::size_t boundary_pos = request.find(boundary_marker);
		std::size_t boundary_pos_end = boundary_pos;

		if (boundary_pos == std::string::npos) {
		
			boundary_pos = request.find(closing_boundary_marker);
			boundary_pos_end = boundary_pos + closing_boundary_marker.size();

			if (boundary_pos == std::string::npos) {
				std::cerr << "400 Bad Request - no end boundary" << std::endl; return 400;
			}
		}

		buffer = request.substr(0, boundary_pos);
		multipart_form_data.set_content(buffer);

		request.erase(0, boundary_pos_end);
		multipartFormDatas.push_back(multipart_form_data);

		multipart_form_data.print_all_data();
	}

	for (auto data : multipartFormDatas)
	{
		// data.print_all_data();
		if (!data.get_filename().empty())
		{
			std::fstream fout(data.get_filename(), std::ios::binary | std::ios::out);
			fout.write(data.get_content().c_str(), data.get_content().size());
			fout.close();
		}
	}

	return 200;
}
