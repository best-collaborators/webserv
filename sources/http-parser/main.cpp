#include <iostream>
#include <fstream>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <regex>

constexpr const char *REGEX_HTTP_METHOD = "((^GET|POST|DELETE)[ ]+)";
constexpr const char *REGEX_HTTP_REQUEST_TARGET = "(^(\\/.+)[ ]+)";
constexpr const char *REGEX_HTTP_VESRION = "(HTTP\\/1.1$)";

constexpr const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
constexpr const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
constexpr const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

std::string get_regex_value(std::string &line, std::regex regex_method)
{
	std::smatch matches;
	std::regex_search(line, matches, regex_method);
	if (matches.empty()) return "";
	std::string matched_string = matches.str(1);
	line = matches.suffix();
	return matched_string;
}

bool add_value_to_map(
	const char *regex_str,
	std::string &line,
	std::string errmsg,
	std::unordered_map<std::string, std::string> &httpRequestValues,
	std::string key
)
{
	std::regex reg_method(regex_str);
	std::string method = get_regex_value(line, reg_method);
	if (method == "") {
		std::cout << errmsg << std::endl;
		return false;
	}
	httpRequestValues[key] = method;
	return true;
}

bool is_valid_request_line(std::string line, std::unordered_map<std::string, std::string> &httpRequestValues)
{
	return (add_value_to_map(REGEX_HTTP_METHOD, line, ERROR_HTTP_METHOD, httpRequestValues, "method")
	&& add_value_to_map(REGEX_HTTP_REQUEST_TARGET, line, ERROR_HTTP_REQUEST_TARGET, httpRequestValues, "request_target")
	&& add_value_to_map(REGEX_HTTP_VESRION, line, ERROR_HTTP_VESRION, httpRequestValues, "version"));
}

static inline std::string &ltrim(std::string &s)
{
	auto is_space = [](int c) {return std::isspace(c);};
	auto not_space_iterator = std::find_if_not(s.begin(), s.end(), is_space);
	s.erase(s.begin(), not_space_iterator);
	return s;
}

static inline std::string &rtrim(std::string &s)
{
	auto is_space = [](int c) {return !std::isspace(c);};
	auto not_space_iterator = std::find_if(s.rbegin(), s.rend(), is_space);
	s.erase(not_space_iterator.base(), s.end());
	return s;
}

static inline std::string &trim(std::string &s)
{
	s = ltrim(s);
	s = rtrim(s);
	return s;
}

bool is_valid_header(std::string line, std::unordered_map<std::string, std::string> &httpRequestValues)
{
	std::regex reg_method("(^.+:[ ]+)");
	std::string name = get_regex_value(line, reg_method);
	if (name == "") return false;
	trim(name);
	name.erase(name.length() - 1);
	std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
	if (!httpRequestValues[name].empty()) {
		if (name == "host" || name == "content-length") {
			std::cerr << "ERR: HEADER DUPLICATION: " << name << std::endl;
			return false;
		}
		else {
			if (httpRequestValues[name] != line || httpRequestValues[name] != trim(line))
			httpRequestValues[name] = httpRequestValues[name] + "," + line;
			return true;
		}
	}
	httpRequestValues[name] = trim(line);
	return true;
}

void print_http_request_values(std::unordered_map<std::string, std::string> httpRequestValues)
{
	for (auto values : httpRequestValues) {
		std::cout << "[" << values.first << "] " << values.second << std::endl;
	}
}

bool is_content_length_valid(
	std::unordered_map<std::string, std::string> &http_request_values
)
{
	if (!http_request_values["forward-encoding"].empty()) {
		std::cerr << "ERR: FORWARD-ENCODING + CONTENT LENGTH" << std::endl;
		return false;
	}
	try {
		size_t pos;
		int test_length = std::stoi(http_request_values["content-length"], &pos, 10);
		// max size is 1mb = 1048576b
		if (http_request_values["content-length"].length() != pos
			|| test_length < 0 || test_length > 1048576) throw std::runtime_error("ERR: INVALID CONTENT LENGTH");
	}
	catch(const std::exception& e) {
		std::cerr << "ERR: INVALID CONTENT LENGTH" << '\n';
		return false;
	}
	return true;
}

int main()
{
	std::string buffer;
	std::unordered_map<std::string, std::string> http_request_values;

	std::ifstream ifs("./sources/http-parser/test_http_request_get.txt");
	if (ifs.fail()) std::cout << std::strerror(errno) << std::endl;

	getline(ifs, buffer);
	if (is_valid_request_line(buffer, http_request_values) == false) return 2;
	while (getline(ifs, buffer)) {
		if (!is_valid_header(buffer, http_request_values)) {
			std::cerr<<"errr"<<std::endl; return 2;
		}
		if (http_request_values["content-length"] != "") {
			if (!is_content_length_valid(http_request_values)) return 2;
		}
	}
	ifs.close();
	print_http_request_values(http_request_values);
	return 0;
}
