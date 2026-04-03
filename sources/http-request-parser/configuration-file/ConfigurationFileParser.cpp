#include "ConfigurationFileParser.hpp"

ConfigurationFileParser::ConfigurationFileParser(std::string filename, server_block_map &server_blocks) : _filename(filename), _server_blocks(server_blocks)
{
	_current_server_block._assigned_fields.reset();
}

bool isEmptyLine(std::string line)
{
	if (line.empty()) return true;
	std::regex reg(HttpRegexPatterns::WHITESPACE());
	return std::regex_match(line, reg);
}

std::string ConfigurationFileParser::_extractDirectiveValue(std::string &line, size_t keyword_length)
{
	line.erase(0, keyword_length);
	Trimmer::trim(line);
	line.erase(0, 1); // remove ':'
	Trimmer::trim(line);
	return line;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseMaxBodySize(std::string &line)
{
	try {
		size_t pos = 0;
		std::stoll(line, &pos, 10);
		if (pos != line.size()) throw std::logic_error("Body size has wrong format");
	}
	catch(const std::exception& e) {
		std::cerr << "Body size is invalid. (Correct format:  max_body_size: 100)" << std::endl;
		return ERROR;
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseRoot(std::string &line)
{
	std::filesystem::path validated_path;
	try {
		_extractDirectiveValue(line, 4);
		validated_path = std::filesystem::weakly_canonical(line);
		if (!std::filesystem::is_directory(validated_path)) throw std::logic_error("Root is invalid");
	}
	catch(const std::exception& e) {
		std::cerr << "Root is invalid. (Correct format:  root: /path_to_root_dir)" << std::endl;
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::DEBUG, "Root: " + line, "config");
	_current_server_block._root = std::filesystem::weakly_canonical(line);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseIndex(std::string &line)
{
	try {
		_extractDirectiveValue(line, 5);
		line = RegexMatcher::get_regex_value(line, HttpRegexPatterns::INDEX(), 0);
		if (line.empty()) {
			throw std::logic_error("Index path is invalid");
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Index is invalid. Correct format:  index: /data/index.html" << std::endl;
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::DEBUG, "Root index: " + line, "config");
	_current_server_block._index = line;
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseErrorPages(std::string &line)
{
	std::string copy = line;
	std::string status_code_str = RegexMatcher::get_regex_value(copy, HttpRegexPatterns::GET_ERROR_PAGE(), 1);
	std::string er_page_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::GET_ERROR_PAGE(), 2);

	HttpStatus::e_code status_code;
	std::string path;
	try {
		path = er_page_str;
		status_code = HttpStatus::e_code(std::stoi(status_code_str));
		if (HttpStatus::is_good(status_code)) throw std::logic_error("Invalid code"); 
	}
	catch(const std::exception& e) {
		std::cerr << "Error page status code is invalid. Correct format:  400: /BadRequestPage.html" << std::endl;
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::DEBUG, "Error page: " + er_page_str + "Status code: " + status_code_str, "config");

	if (_current_server_block._error_pages.count(status_code)) {
		std::cerr << "Error page status code is duplicate" << std::endl;
		return ERROR;
	}
	_current_server_block._error_pages[status_code] = path;
	return OK;
}

bool isValidHeaderFormat(const std::string& line,
						 const std::string& header_name,
						 bool one_line)
{
	std::string pattern;

	if (one_line)
		pattern = "^\\s*" + header_name + "\\s*:\\s*\\S.*$";
	else
		pattern = "^\\s*" + header_name + "\\s*:\\s*$";

	std::regex reg(pattern);
	return std::regex_match(line, reg);
}

namespace {
	bool isValidTokenChar(char c) {
		return std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-';
	}

	std::unordered_set<std::string> split(const std::string& s)
	{
		std::unordered_set<std::string> result;
		size_t start = 0;
		for (size_t i = 0; i <= s.size(); i++)
		{
			if (i == s.size() || s[i] == '|')
			{
				std::string token = s.substr(start, i - start);
				// Trim leading and trailing whitespace
				size_t first = token.find_first_not_of(" \t");
				size_t last = token.find_last_not_of(" \t");

				if (first != std::string::npos)
				{
					std::string trimmed = token.substr(first, last - first + 1);
					// Check that all characters are valid
					bool is_valid = !trimmed.empty() &&
					                 std::all_of(trimmed.begin(), trimmed.end(), isValidTokenChar);
					if (!is_valid)
					{
						result.clear(); // invalid token found
						break;
					}
					// Insert only if not already present
					if (result.count(trimmed))
					{
						result.clear();
						break;
					}
					result.insert(trimmed);
				}
				start = i + 1;
			}
		}
		return result;
	}
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_updateAllowedMethods(std::string &methods_str, HttpMethodRegistry &methods_registry)
{
	Trimmer::trim(methods_str, '"');
	std::stringstream ss;
	ss << methods_str;
	std::string temp;
	while (getline(ss, temp, '|')) {
		Trimmer::trim(temp);
		if (temp != "GET" && temp != "DELETE" && temp != "POST" && temp != "OPTIONS")
			return ERROR; 
		methods_registry.setAllowedMethod(HttpMethod::fromString(temp));
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseAllowedMethods(std::string &line, std::optional<HttpMethodRegistry> &registry, std::bitset<8> &fields)
{
	if (_checkDuplicateField(0, "allowed_methods", fields) == ERROR) return ERROR;

	_extractDirectiveValue(line, 15);
	std::string methods_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::ALLOWED_METHODS(), 0);
	if (methods_str.empty()) {
		std::cerr << "Allowed methods are invalid. Correct format examples:\n1."
			"allowed_methods: \"GET|POST\"\n2. allowed_methods: \"POST\"" << std::endl;
		return ERROR;
	}
	if (!registry.has_value())
		registry.emplace();
	if (registry.has_value())
		_updateAllowedMethods(methods_str, registry.value());
	fields.set(0);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGIExtensions(std::string &line, CGIPath &cgi, std::bitset<8> &fields)
{
	try {
		_extractDirectiveValue(line, 10);
		Trimmer::trim(line);
		Trimmer::trim(line, '"');
		cgi.extensions = split(line);

		for (const auto &ex: cgi.extensions) {
			if (ex[0] != '.') {
				throw std::logic_error("Extensions are invalid");
			}
		}
		if (cgi.extensions.empty()) {
			throw std::logic_error("Extensions are invalid");
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Extensions are invalid. "
			"Correct format examples:\n1. extensions: \".js|.py\"\n2. extensions: \".py\"" << std::endl;
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::DEBUG, to_string(cgi.extensions), "config");
	fields.set(2);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGIMaxBodySize(
	std::string &line, CGIPath &cgi, std::bitset<8> &fields)
{
	if (_checkDuplicateField(5, "max_body_size", fields) == ERROR) return ERROR;
	_extractDirectiveValue(line, 13);

	if (_parseMaxBodySize(line) == ERROR) return ERROR;
	cgi.max_body_size = std::stoll(line);
	fields.set(5);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_dispatchCGIDirective(
	std::string &line, CGIPath & cgi, std::bitset<8> &fields)
{
	if (isValidHeaderFormat(line, "allowed_methods", true))
		return _parseAllowedMethods(line, cgi.methods_registry, fields);
	if (isValidHeaderFormat(line, "extensions", true))
		return _parseCGIExtensions(line, cgi, fields);
	if (isValidHeaderFormat(line, "max_body_size", true))
		return _parseCGIMaxBodySize(line, cgi, fields);

	std::cerr << "Invalid field: " + line << std::endl;
	return ERROR;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGI(std::ifstream &ifs, std::string &line)
{
	_extractDirectiveValue(line, 0);
	if (!isValidHeaderFormat(line, "pass_to", true)) return ERROR;

	_extractDirectiveValue(line, 7);
	std::string pass_to = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (pass_to.empty()) {
		std::cerr << "CGI pass_to is invalid. Correct format: - pass_to: /pass/to/executable" << std::endl;
		return ERROR;
	}

	std::bitset<8> cgi_assigned_fields;
	CGIPath cgi;
	cgi.pass_to = std::filesystem::weakly_canonical(pass_to);

	if (access(cgi.pass_to.c_str(), X_OK) || std::filesystem::is_directory(cgi.pass_to)){
		std::cerr << "pass_to is not executable: " + cgi.pass_to << std::endl;
		return ERROR;
	}
	Logger::displayLog(Logger::e_log_level::DEBUG, "Pass to executable: " + cgi.pass_to, "config");

	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (_dispatchCGIDirective(line, cgi, cgi_assigned_fields) == ERROR)
			return ERROR;

		if (_isStreamFinished(ifs)) break;
	}

	if (!cgi_assigned_fields.test(2)) {
		std::cerr << "Missing extensions for CGI" << std::endl;
		return ERROR;
	}

	if (!_current_server_block._cgi.has_value())
		_current_server_block._cgi.emplace();

	_current_server_block._cgi->push_back(cgi);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocationIndex(
	std::string &line, Location &location, std::bitset<8> &fields)
{
	if (_checkDuplicateField(1, "index", fields) == ERROR) return ERROR;
	_extractDirectiveValue(line, 5);

	std::string index_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::INDEX(), 0);
	try
	{
		if (index_str.empty()) {
			throw std::logic_error("Index path is invalid");
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	location.setDefaultFile(index_str);
	fields.set(1);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocationRoot(
	std::string &line, Location &location, std::bitset<8> &fields)
{
	if (_checkDuplicateField(2, "root", fields) == ERROR) return ERROR;
	_extractDirectiveValue(line, 4);

	location.setRoot(line);
	fields.set(2);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocationRedirect(
	std::string &line, Location &location, std::bitset<8> &fields)
{
	if (_checkDuplicateField(3, "redirect", fields) == ERROR) return ERROR;
	_extractDirectiveValue(line, 8);

	HttpPage page;
	try {
		page.status_code = HttpStatus::e_code(std::stoi(line.substr(0, 3)));
		line.erase(0, 3);
		page.path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
		if (page.path.empty()) {
			std::cerr << "Redirection path is invalid. Correct format: redirect: 301 /new_location_path" << std::endl;
			return ERROR;
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Error page status code is invalid" << std::endl;
		return ERROR;
	}
	location.setReturnPage(page);
	fields.set(3);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocationAutoindex(
	std::string &line, Location &location, std::bitset<8> &fields)
{
	if (_checkDuplicateField(4, "autoindex", fields) == ERROR) return ERROR;
	_extractDirectiveValue(line, 9);

	if (line != "on" && line != "off") {
		std::cerr << "Autoindex is invalid. "
			"Correct format: \n1. autoindex: on\n2. autoindex: off" << std::endl;
		return ERROR;
	}

	location.setAutoindex(line == "on" ? true : false);
	fields.set(4);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocationMaxBodySize(
	std::string &line, Location &location, std::bitset<8> &fields)
{
	if (_checkDuplicateField(5, "max_body_size", fields) == ERROR) return ERROR;
	_extractDirectiveValue(line, 13);

	if (_parseMaxBodySize(line) == ERROR) return ERROR;
	location.setMaxBodySize(std::stoll(line));
	fields.set(5);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_dispatchLocationDirective(
	std::string &line, Location &location, std::bitset<8> &fields)
{
	if (isValidHeaderFormat(line, "allowed_methods", true)) {
		std::optional<HttpMethodRegistry> method_registry;
		e_parse_result result = _parseAllowedMethods(line, method_registry, fields);
		if (method_registry.has_value())
			location.setMethodsRegistry(method_registry.value());
		
		return result;
	}
	if (isValidHeaderFormat(line, "index", true))
		return _parseLocationIndex(line, location, fields);
	if (isValidHeaderFormat(line, "root", true))
		return _parseLocationRoot(line, location, fields);
	if (isValidHeaderFormat(line, "redirect", true))
		return _parseLocationRedirect(line, location, fields);
	if (isValidHeaderFormat(line, "autoindex", true))
		return _parseLocationAutoindex(line, location, fields);
	if (isValidHeaderFormat(line, "max_body_size", true))
		return _parseLocationMaxBodySize(line, location, fields);

	std::cerr << "Invalid field: " + line << std::endl;
	return ERROR;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocations(std::ifstream &ifs, std::string &line)
{
	_extractDirectiveValue(line, 0);
	_extractDirectiveValue(line, 4);
	std::string path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (path.empty()) {
		std::cerr << "Location path is invalid. Correct format: - path: /location_path" << std::endl;
		return ERROR;
	}

	std::bitset<8> location_assigned_fields;
	Location location;
	location.setPath(std::filesystem::weakly_canonical(path));

	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (_dispatchLocationDirective(line, location, location_assigned_fields) == ERROR)
			return ERROR;
		if (_isStreamFinished(ifs)) break;
	}

	if (!_current_server_block._locations.has_value())
		_current_server_block._locations.emplace();

	if (location.getPath() == "/") {
		_current_server_block._root_restrictions = location;
	}
	_current_server_block._locations->push_back(location);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseListen(std::string &line)
{
	std::string copy = line;
	std::string ip_addr;
	std::string port;

	ip_addr = RegexMatcher::get_regex_value(copy, HttpRegexPatterns::IP_ADDR_PORT(), 2);
	if (!ip_addr.empty())
		port = RegexMatcher::get_regex_value(line, HttpRegexPatterns::IP_ADDR_PORT(), 3);

	if (ip_addr.empty() || port.empty()) {
		std::cerr << "Invalid listen parameters. Correct format: 127.0.0.1:8000" << std::endl;
		return ERROR;
	}

	_current_server_block._listen_data.ip_address = ip_addr;
	try {
		size_t pos;
		_current_server_block._listen_data.port = std::stoi(port, &pos);
		if (pos != port.size()) {
			throw std::logic_error("Port is invalid");
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Port is invalid: " + port << std::endl;
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::DEBUG, "Ip address: " + ip_addr, "config");
	Logger::displayLog(Logger::e_log_level::DEBUG, "Port: " + port, "config");
	return OK;
}

bool ConfigurationFileParser::_isStreamGood(std::ifstream &ifs)
{
	if (ifs.bad() && !ifs.eof()) {
		std::cerr << "Error happened while reading " + _filename << std::endl;
		return false;
	}
	return true;
}

bool ConfigurationFileParser::_isStreamFinished(std::ifstream &ifs)
{
	return (ifs.eof());
}

bool ConfigurationFileParser::_validateAndConsumeIndent(std::string &line, size_t intend_level, char c, bool show_msg)
{
	std::string checker(intend_level, c);
	if (line.size() < intend_level || line.compare(0, intend_level, checker) != 0 || line[intend_level] == '\t') {
		if (show_msg)
			std::cerr << "Invalid field: " + line << std::endl;
		return false;
	}
	line.erase(0, intend_level);
	return true;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateServerBlocks()
{
	for (auto &s_block : _server_blocks)
	{
		if (_validateRequiredFields(s_block.second) == ERROR) return ERROR;
		if (_validateIndexPath(s_block.second) == ERROR) return ERROR;
		if (_validateErrorPages(s_block.second) == ERROR) return ERROR;
		if (_validateLocations(s_block.second) == ERROR) return ERROR;
		if (_validateCGI(s_block.second) == ERROR) return ERROR;

		#ifdef DDEBUG_FLAG
			std::cout << s_block.second << std::endl;
		#endif
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateRequiredFields(const ServerBlock &s_block)
{
	static const std::vector<std::pair<short, std::string>> required = {{0, "listen"}, {4, "root"}};
	for (auto &p : required) {
		if (!s_block._assigned_fields.test(p.first)) {
			std::cerr << "Missing field " + p.second << std::endl;
			return ERROR;
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateIndexPath(const ServerBlock &s_block)
{
	if (!s_block._index.has_value()) return OK;

	std::filesystem::path full_index_path = s_block._root.string() + "/" + s_block._index.value();
	full_index_path = std::filesystem::weakly_canonical(full_index_path);
	if (full_index_path.string().find(s_block._root) == std::string::npos) {
		Logger::displayLog(Logger::e_log_level::CRITICAL, "File escapes root directory", "config");
		return ERROR;
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateErrorPages(ServerBlock &s_block)
{
	for (auto &err_page : s_block._error_pages) {
		err_page.second = s_block._root.string() + err_page.second;
		std::filesystem::path full = std::filesystem::weakly_canonical(err_page.second);
		if (full.string().find(s_block._root) == std::string::npos) {
			Logger::displayLog(Logger::e_log_level::CRITICAL, "File escapes root directory.", "config");
			return ERROR;
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateLocations(ServerBlock &s_block)
{
	if (!s_block._locations.has_value()) return OK;

	s_block._root_restrictions.setRoot(s_block._root);
	std::unordered_set<std::string> seen_paths;

	for (auto &l : s_block._locations.value()) {

		std::string path = l.getPath();
		if (seen_paths.count(path)) {
			std::cerr << "Duplicate location path: " + path << std::endl;
			return ERROR;
		}
		seen_paths.insert(path);

		std::filesystem::path root;

		if (l.getRoot().empty()) {
			root = s_block._root.string() + l.getPath().string();
		} else {
			root = s_block._root / l.getRoot();
		}

		root = std::filesystem::weakly_canonical(root);
		l.setRoot(root);

		std::filesystem::path full = std::filesystem::weakly_canonical(l.getRoot().string() + "/" + l.getDefaultFile());
		if (full.string().find(s_block._root) == std::string::npos) {
			Logger::displayLog(Logger::e_log_level::CRITICAL, "File escapes root directory", "config");
			return ERROR;
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateCGI(ServerBlock &s_block)
{
	if (!s_block._cgi.has_value()) return OK;
	std::unordered_set<std::string> seen_ext;

	for (auto &cgi : s_block._cgi.value()) {
		for (auto &ext : cgi.extensions) {
			if (seen_ext.count(ext)) {
				std::cerr << "Duplicate cgi extension: " + ext << std::endl;
				return ERROR;
			}
			seen_ext.insert(ext);
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_checkDuplicateField(
	size_t bit_index, const std::string &field_name, std::bitset<8UL> &assigned_fields)
{
	if (assigned_fields.test(bit_index)) {
		std::cerr << "Double field " + field_name << std::endl;
		return ERROR;
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleListenDirective(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(0, "listen", _current_server_block._assigned_fields) == ERROR) return ERROR;

	getline(ifs, line);
	if (!_isStreamGood(ifs)) return ERROR;
	if (_isStreamFinished(ifs)) return OK;
	if (!_validateAndConsumeIndent(line, 2, '\t')) return ERROR;
	if (_parseListen(line) == ERROR) return ERROR;

	extra_line = false;
	_current_server_block._assigned_fields.set(0);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleErrorPagesDirective(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(2, "error_pages", _current_server_block._assigned_fields) == ERROR) return ERROR;

	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 2, '\t', false)) {
			extra_line = true;
			break;
		}
		if (_parseErrorPages(line) == ERROR) return ERROR;
		if (_isStreamFinished(ifs)) break;
	}
	_current_server_block._assigned_fields.set(2);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleRootDirective(std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(4, "root", _current_server_block._assigned_fields) == ERROR) return ERROR;

	if (_parseRoot(line) == ERROR) return ERROR;
	extra_line = false;
	_current_server_block._assigned_fields.set(4);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleIndexDirective(std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(5, "index", _current_server_block._assigned_fields) == ERROR) return ERROR;

	if (_parseIndex(line) == ERROR) return ERROR;
	extra_line = false;
	_current_server_block._assigned_fields.set(5);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleLocationsDirective(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(6, "locations", _current_server_block._assigned_fields) == ERROR) return ERROR;

	getline(ifs, line);
	while (true)
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (isEmptyLine(line)) { getline(ifs, line); continue; }
		if (!_validateAndConsumeIndent(line, 2, '\t', false)) {
			extra_line = true;
			break;
		}
		if (_parseLocations(ifs, line) == ERROR) return ERROR;
		if (_isStreamFinished(ifs)) break;
	}

	#ifdef DDEBUG_FLAG
		if (_current_server_block._locations.has_value()) {
			for (auto &l : _current_server_block._locations.value()) {
				std::cout << l << std::endl;
			}
		}
	#endif

	_current_server_block._assigned_fields.set(6);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleCGIDirective(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(7, "cgi", _current_server_block._assigned_fields) == ERROR) return ERROR;

	getline(ifs, line);
	while (true)
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) { getline(ifs, line); continue; }
		if (!_validateAndConsumeIndent(line, 2, '\t', false)){
			extra_line = true;
			break;
		}
		if (_parseCGI(ifs, line) == ERROR) return ERROR;
	}

	#ifdef DDEBUG_FLAG
		if (_current_server_block._cgi.has_value()) {
			for (auto &cgi : _current_server_block._cgi.value()) {
				std::cout << cgi << std::endl;
			}
		}
	#endif

	_current_server_block._assigned_fields.set(7);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_dispatchServerDirective(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	if (isValidHeaderFormat(line, "listen", false))
		return _handleListenDirective(ifs, line, extra_line);

	if (isValidHeaderFormat(line, "error_pages", false))
		return _handleErrorPagesDirective(ifs, line, extra_line);

	if (isValidHeaderFormat(line, "root", true))
		return _handleRootDirective(line, extra_line);

	if (isValidHeaderFormat(line, "index", true))
		return _handleIndexDirective(line, extra_line);

	if (isValidHeaderFormat(line, "locations", false))
		return _handleLocationsDirective(ifs, line, extra_line);

	if (isValidHeaderFormat(line, "cgi", false))
		return _handleCGIDirective(ifs, line, extra_line);

	std::cerr << "Invalid field name: " + line << std::endl;
	return ERROR;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseSingleServerBlock(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	extra_line = false;

	while (true)
	{
		if (!extra_line)
			getline(ifs, line);

		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 1, '\t', false)) {
			extra_line = true;
			break;
		}

		e_parse_result result = _dispatchServerDirective(ifs, line, extra_line);
		if (result == ERROR)
			return ERROR;
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseAllServerBlocks(std::ifstream &ifs)
{
	std::string line;
	bool extra_line = false;

	while (true)
	{
		if (!extra_line)
		{
			if (!std::getline(ifs, line))
				break;
		}

		Logger::displayLog(Logger::e_log_level::DEBUG, "Current field: " + line, "config");
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 0, '\t')) return ERROR;

		if (isValidHeaderFormat(line, "server", false))
		{
			_current_server_block = ServerBlock();
			e_parse_result result = _parseSingleServerBlock(ifs, line, extra_line);
			if (result == ERROR)
				return ERROR;

			Trimmer::trim(line, '\t');
			Trimmer::trim(line);
			if (_isStreamFinished(ifs) && !line.empty()) {
				std::cerr << "Invalid EOF" << std::endl;
				return ERROR;
			}
			auto [it, inserted] = _server_blocks.emplace(_current_server_block._listen_data, _current_server_block);

			if (!inserted) {
				std::cerr <<  "Duplicate (port,id) detected" << std::endl;
				return ERROR;
			}
		}
		else
		{
			std::cerr << "Invalid field name: " + line << std::endl;
			return ERROR;
		}
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::parse()
{
	if (_filename.empty()) _filename = "default.conf";
	std::ifstream ifs(_filename);
	if (!ifs.good()) {
		std::cerr << "Cannot access " + _filename << std::endl;
		return ERROR;
	}

	e_parse_result result = _parseAllServerBlocks(ifs);
	if (result == ERROR)
		return ERROR;

	return _validateServerBlocks();
}
