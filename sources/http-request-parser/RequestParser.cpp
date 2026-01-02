#include "RequestParser.hpp"

RequestParser::MultipartFormData::MultipartFormData(std::string content_type, std::string name, std::string filename)
: _content_type(content_type), _name(name), _filename(filename) {

	std::cout << _content_type <<" "<<_name << " " << _filename << std::endl;
 }

 RequestParser::MultipartFormData::MultipartFormData() {}

RequestParser::RequestParser(std::unordered_map<std::string, std::string> &http_request_values)
: _http_request_values(http_request_values) { }

inline std::string &RequestParser::ltrim(std::string &s)
{
	auto is_space = [](int c) {return std::isspace(c);};
	auto not_space_iterator = std::find_if_not(s.begin(), s.end(), is_space);
	s.erase(s.begin(), not_space_iterator);
	return s;
}

void RequestParser::MultipartFormData::print_all_data()
{
	std::cout << "Content-Type: " << _content_type << std::endl;
	std::cout << "Name: " << _name << std::endl;
	std::cout << "Filename: " << _filename << std::endl;
	std::cout << "Content: " << _content << std::endl;
}

const std::string &RequestParser::MultipartFormData::get_content_type() const {
	return _content_type;
}

void RequestParser::MultipartFormData::set_content_type(const std::string &content_type) {
	_content_type = content_type;
}

const std::string &RequestParser::MultipartFormData::get_name() const {
	return _name;
}

void RequestParser::MultipartFormData::set_name(const std::string &name) {
	_name = name;
}

const std::string &RequestParser::MultipartFormData::get_filename() const {
	return _filename;
}

const std::string &RequestParser::MultipartFormData::get_content() const {
	return _content;
}

void RequestParser::MultipartFormData::set_filename(const std::string &filename) {
	_filename = filename;
}

void RequestParser::MultipartFormData::set_content(const std::string &content) {
	_content = content;
}

void RequestParser::MultipartFormData::append_content(const std::string &content) {
	if (!content.empty())
		_content += content + "\n";
}

inline std::string &RequestParser::rtrim(std::string &s)
{
	auto is_space = [](int c) {return std::isspace(c);};
	auto not_space_iterator = std::find_if_not(s.rbegin(), s.rend(), is_space);
	s.erase(not_space_iterator.base(), s.end());
	return s;
}

inline std::string &RequestParser::trim(std::string &s)
{
	s = ltrim(s);
	s = rtrim(s);
	return s;
}

inline std::string &RequestParser::ltrim(std::string &s, char delim)
{
	auto is_delim = [delim](int c) { return c == delim; };
	auto not_delim_iterator = std::find_if_not(s.begin(), s.end(), is_delim);
	s.erase(s.begin(), not_delim_iterator);
	return s;
}

inline std::string &RequestParser::rtrim(std::string &s, char delim)
{
	auto is_delim = [delim](int c) {return c == delim; };
	auto not_delim_iterator = std::find_if_not(s.rbegin(), s.rend(), is_delim);
	s.erase(not_delim_iterator.base(), s.end());
	return s;
}

inline std::string &RequestParser::trim(std::string &s, char delim)
{
	s = ltrim(s, delim);
	s = rtrim(s, delim);
	return s;
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
	_http_request_values[key] = trim(method);
	return true;
}

bool RequestParser::is_valid_header()
{
	std::regex reg_method("(^\\S{1,256}:[ ]+)");
	std::string name = get_regex_value(buffer, reg_method);
	if (name == "") return false;
	
	trim(buffer);
	if (buffer.length() > 8192) return false;

	trim(name);
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
	trim(boundary);
	trim(boundary, '\"');
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

bool RequestParser::check_multipart_header(RequestParser::MultipartFormData &multipart_form_data)
{
	std::regex reg("^[C,c]ontent-[D,d]isposition: form-data;\\s*name=\"(\\S{1,256})\";?\\s*(filename=\"(\\S{1,256})\")?");
	std::smatch m;
	if (!std::regex_search(buffer, m, reg)) { return false; }

	multipart_form_data.set_name(m[1]);
	multipart_form_data.set_filename(m[2]);
	return true;
}

bool RequestParser::check_multipart_content_type(RequestParser::MultipartFormData &multipart_form_data)
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

uint RequestParser::get_status_code()
{
	buffer = "";

	std::string request_file = "./sources/http-request-parser/test_http_request.txt";
	std::ifstream ifs(request_file);
	if (ifs.fail())
	{
		std::cout << std::strerror(errno) << " file: " << request_file << std::endl;
		ifs.close(); return -1;
	}

	getline(ifs, buffer);
	if (is_valid_request_line() == false){
		std::cerr << "400 Bad Request" << std::endl;
		ifs.close(); return 400;
	}

	if (_http_request_values["version"] != "HTTP/1.1") {
		std::cerr << "505 HTTP Version Not Supported" << std::endl;
		ifs.close(); return 505;
	}

	if (_http_request_values["request-target"].length() > 4096){
		std::cerr << "414 URI Too Long" << std::endl;
		ifs.close(); return 414;
	}

	if (_http_request_values["method"] != "GET"
		&& _http_request_values["method"] != "POST"
		&& _http_request_values["method"] != "DELETE") {
		std::cerr << "405 Not Allowed" << std::endl;
		ifs.close(); return 405;
	}

	while (getline(ifs, buffer)) {

		if (buffer == "" && ifs.good()) break;

		if (!is_valid_header()) {
			std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
		}
		if (_http_request_values.size() >= 256) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl; ifs.close(); return 400;
		}
		if (_http_request_values.count("content-length")) {
			if (int status_code = content_length_validation()) {
				ifs.close(); return status_code;
			}
		}
	}

	if (_http_request_values["method"] == "POST" && !_http_request_values.count("content-length")) {
		std::cerr << "411 Length Required" << std::endl; ifs.close(); return 411;
	}

	std::string boundary;
	if (_http_request_values["method"] == "POST" && _http_request_values["content-type"].find("multipart/form-data") != std::string::npos) {
		boundary = get_multipart_form_boundary();
		if (boundary.empty()) {
			std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
		}
	}

	//if post and content length is not 0 - error!
	getline(ifs, buffer);
	if (buffer == "" && ifs.bad()) {
		if (_http_request_values["method"] == "POST" && _http_request_values.count("content-length")) {
			std::cerr << "Body required!" << std::endl;
			return 400;
		}
		ifs.close(); return 200;
	}

	std::vector<MultipartFormData> multipartFormDatas;
	if (!boundary.empty())
	{
		while (ifs.good())
		{
			if (buffer == "--" + boundary) {

				MultipartFormData m_data("", "", "");

				getline(ifs, buffer);
				if (!check_multipart_header(m_data)) {
					std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
				}
				
				getline(ifs, buffer);
				if (!check_multipart_content_type(m_data)) {
					std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
				}
				
				if (!buffer.empty())
				{
					getline(ifs, buffer);
					if (!buffer.empty() || (buffer.empty() && ifs.bad())) {
						std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
					}
				}

				do
				{
					if (buffer == "--" + boundary || buffer == "--" + boundary + "--" || ifs.bad()) break;
					m_data.append_content(buffer);
				} while (getline(ifs, buffer));

				// m_data.set_content(m_data.get_content().erase(m_data.get_content().end() - 1, m_data.get_content().end()));
				m_data.print_all_data();
				multipartFormDatas.push_back(m_data);
				if (buffer == "--" + boundary + "--") break;
			}
			else {
				std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
			}
		}

		getline(ifs, buffer);
		if (!buffer.empty()) {
			std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
		}

		getline(ifs, buffer);
		if (ifs.eof()) {
			std::cerr << "201 Created" << std::endl; ifs.close(); return 201;
		}
		std::cerr << "400 Bad Request" << std::endl; ifs.close(); return 400;
	}

	std::string body;
	while (getline(ifs, buffer))
	{
		body += buffer + "\n";
	}


	std::cout << body;
	ifs.close();
	return 200;
}

// int RequestParser::parse_multipart_form()
// {
// }
