#include "RequestParser.hpp"

RequestParser::RequestParser(std::unordered_map<std::string, std::string> &http_request_values)
: _http_request_values(http_request_values) {}

inline std::string &RequestParser::ltrim(std::string &s)
{
	auto is_space = [](int c) {return std::isspace(c);};
	auto not_space_iterator = std::find_if_not(s.begin(), s.end(), is_space);
	s.erase(s.begin(), not_space_iterator);
	return s;
}

inline std::string &RequestParser::rtrim(std::string &s)
{
	auto is_space = [](int c) {return !std::isspace(c);};
	auto not_space_iterator = std::find_if(s.rbegin(), s.rend(), is_space);
	s.erase(not_space_iterator.base(), s.end());
	return s;
}

inline std::string &RequestParser::trim(std::string &s)
{
	s = ltrim(s);
	s = rtrim(s);
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
	if (!_http_request_values["forward-encoding"].empty()) {
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

uint RequestParser::get_status_code()
{
	buffer = "";

	std::string request_file = "./sources/http-parser/test_http_request.txt";
	std::ifstream ifs(request_file);
	if (ifs.fail())
	{
		std::cout << std::strerror(errno) << " file: " << request_file << std::endl;
		return -1;
	}

	getline(ifs, buffer);
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

	while (getline(ifs, buffer)) {
		if (!is_valid_header()) {
			std::cerr << "400 Bad Request" << std::endl; return 400;
		}
		if (_http_request_values.size() >= 256) {
			std::cerr << "431 Request Header Fields Too Large" << std::endl; return 400;
		}
		if (!_http_request_values["content-length"].empty()) {
			if (int status_code = content_length_validation()) {
				return status_code;
			}
		}
	}

	if (_http_request_values["method"] == "POST" && _http_request_values["content-length"].empty()) {
		std::cerr << "411 Length Required" << std::endl; return 411;
	}

	ifs.close();
	return 200;
}
