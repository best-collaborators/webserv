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
		Logger::displayLog(Logger::e_log_level::ERROR, "Body size is invalid: " + line, "config");
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
		Logger::displayLog(Logger::e_log_level::ERROR, "Root is invalid: " + line, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Root: " + line, "config");
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
		Logger::displayLog(Logger::e_log_level::ERROR, "Index is invalid: " + line, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Index: " + line, "config");
	_current_server_block._index = line;
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseServerName(std::string &line)
{
	_extractDirectiveValue(line, 11);
	std::string server_name_validated = RegexMatcher::get_regex_value(line, HttpRegexPatterns::NON_WHITESPACE(), 1);

	if (server_name_validated.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Invalid server name", "config");
		return ERROR;
	}
	_current_server_block._server_name = server_name_validated;
	Logger::displayLog(Logger::e_log_level::INFO, "Server name: " + _current_server_block._server_name, "config");
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
		Logger::displayLog(Logger::e_log_level::ERROR, "Error page status code is invalid: " + er_page_str, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Status code: " + status_code_str, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Error page: " + er_page_str, "config");

	if (_current_server_block._error_pages.count(status_code)) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Error page status code is duplicate: " + status_code_str, "config");
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


//TODO: change allowed methods regex to split
namespace {
	std::unordered_set<std::string> split(const std::string& s)
	{
		std::unordered_set<std::string> result;
		size_t start = 0;

		for (size_t i = 0; i <= s.size(); i++)
		{
			if (i == s.size() || s[i] == '|')
			{
				std::string token = s.substr(start, i - start);

				size_t first = token.find_first_not_of(" \t");
				size_t last = token.find_last_not_of(" \t");

				std::cout << *token.begin() << std::endl;
				std::string to_insert = token.substr(first, last - first + 1);
				if (result.count(to_insert)) {
					result.clear();
					break;
				}
				if (first != std::string::npos)
					result.insert(token.substr(first, last - first + 1));

				start = i + 1;
			}
		}
		return result;
	}
}


void ConfigurationFileParser::_updateAllowedMethods(std::string &methods_str, HttpMethodRegistry &methods_registry)
{
	Trimmer::trim(methods_str, '"');
	std::stringstream ss;
	ss << methods_str;
	std::string temp;
	while (getline(ss, temp, '|')) {
		Trimmer::trim(temp);
		methods_registry.setAllowedMethod(HttpMethod::fromString(temp));
	}
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseAllowedMethods(std::string &line, std::optional<HttpMethodRegistry> &registry, std::bitset<8> &fields)
{
	if (_checkDuplicateField(0, "allowed_methods", fields) == ERROR) return ERROR;

	_extractDirectiveValue(line, 15);
	std::string methods_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::ALLOWED_METHODS(), 0);
	if (methods_str.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Allowed methods are invalid: " + line, "config");
		return ERROR;

	}
	if (!registry.has_value())
		registry.emplace();
	_updateAllowedMethods(methods_str, registry.value());
	fields.set(0);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGIPath(std::string &line, CGIPath &cgi, std::bitset<8> &fields)
{
	_extractDirectiveValue(line, 4);
	std::string path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (path.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "CGI path is invalid: " + line, "config");
		return ERROR;
	}

	cgi.path = line;
	fields.set(1);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGIExtensions(std::string &line, CGIPath &cgi, std::bitset<8> &fields)
{
	try {
		_extractDirectiveValue(line, 10);
		Trimmer::trim(line);
		Trimmer::trim(line, '"');
		cgi.extensions = split(line);

		if (cgi.extensions.empty()) {
			throw std::logic_error("Extensions are invalid");
		}
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Extensions are invalid: " + line, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, to_string(cgi.extensions), "config");
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
	if (isValidHeaderFormat(line, "path", true))
		return _parseCGIPath(line, cgi, fields);
	if (isValidHeaderFormat(line, "extensions", true))
		return _parseCGIExtensions(line, cgi, fields);
	if (isValidHeaderFormat(line, "max_body_size", true))
		return _parseCGIMaxBodySize(line, cgi, fields);

	Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field: " + line, "config");
	return ERROR;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGI(std::ifstream &ifs, std::string &line)
{
	_extractDirectiveValue(line, 0);
	_extractDirectiveValue(line, 7);
	std::string pass_to = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (pass_to.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "CGI pass_to is invalid: " + line, "config");
		return ERROR;
	}

	std::bitset<8> cgi_assigned_fields;
	CGIPath cgi;
	cgi.pass_to = std::filesystem::weakly_canonical(pass_to);

	if (access(cgi.pass_to.c_str(), X_OK) || std::filesystem::is_directory(cgi.pass_to)){
		Logger::displayLog(Logger::e_log_level::ERROR, "pass_to is not executable: " + line, "config");
		return ERROR;
	}

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
		Logger::displayLog(Logger::e_log_level::ERROR, "Missing extensions for CGI", "config");
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
		std::cerr << e.what() << '\n';
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

	location.setRoot(std::filesystem::weakly_canonical(line));
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
		page.path = std::filesystem::weakly_canonical(line.substr(3));
		page.status_code = HttpStatus::e_code(std::stoi(line.substr(0, 3)));
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Error page status code is invalid: " + line, "config");
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
		Logger::displayLog(Logger::e_log_level::ERROR, "Autoindex is invalid: " + line, "config");
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

	Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field: " + line, "config");
	return ERROR;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocations(std::ifstream &ifs, std::string &line)
{
	_extractDirectiveValue(line, 0);
	_extractDirectiveValue(line, 4);
	std::string path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (path.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Location path is invalid: " + line, "config");
		return ERROR;
	}

	std::bitset<8> location_assigned_fields;
	Location location;
	location.setPath(std::filesystem::weakly_canonical(path));

	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (_dispatchLocationDirective(line, location, location_assigned_fields) == ERROR)
			return ERROR;
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
	std::string ip_addr = RegexMatcher::get_regex_value(copy, HttpRegexPatterns::IP_ADDR_PORT(), 2);
	std::string port = RegexMatcher::get_regex_value(line, HttpRegexPatterns::IP_ADDR_PORT(), 3);

	if (ip_addr.empty() || port.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Invalid listen parameters", "config");
		return ERROR;
	}

	_current_server_block._listen_data.ip_address = ip_addr;

	try {
		_current_server_block._listen_data.port = std::stoi(port);
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Port is invalid: " + port, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Ip address: " + ip_addr, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Port: " + port, "config");
	return OK;
}

bool ConfigurationFileParser::_isStreamGood(std::ifstream &ifs)
{
	if (ifs.bad() && !ifs.eof()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Error happened while reading " + _filename, "config");
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
			Logger::displayLog(Logger::e_log_level::ERROR, "Invalid format: " + line, "config");
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

		std::cout << s_block.second << std::endl;
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateRequiredFields(const ServerBlock &s_block)
{
	// bits 0=listen, 1=server_name, 4=root are required
	static const std::vector<size_t> required = {0, 1, 4};
	for (size_t i : required) {
		if (!s_block._assigned_fields.test(i)) {
			Logger::displayLog(Logger::e_log_level::ERROR, "Missing field " + std::to_string(i), "config");
			return ERROR;
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateIndexPath(const ServerBlock &s_block)
{
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
			Logger::displayLog(Logger::e_log_level::CRITICAL, "File escapes root directory", "config");
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
			Logger::displayLog(
				Logger::e_log_level::ERROR, "Duplicate location path: " + path, "config");
			return ERROR;
		}
		seen_paths.insert(path);

		if (!std::filesystem::is_directory(l.getRoot())) l.setRoot(s_block._root.string());

		if (!std::filesystem::is_directory(l.getRoot())) {
			Logger::displayLog(Logger::e_log_level::ERROR, "Is not a dir: " + l.getRoot().string(), "config");
			return ERROR;
		}

		std::filesystem::path full = std::filesystem::weakly_canonical(l.getRoot().string() + "/" + l.getDefaultFile());
		if (full.string().find(l.getRoot().c_str(), 0, l.getRoot().string().size() - 1) == std::string::npos) {
			Logger::displayLog(Logger::e_log_level::CRITICAL, "File escapes root directory: " + full.string(), "config");
			return ERROR;
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_validateCGI(ServerBlock &s_block)
{
	if (!s_block._cgi.has_value()) return OK;
	std::unordered_set<std::string> seen_paths;

	for (auto &cgi : s_block._cgi.value()) {

		std::string path = cgi.path;
		if (seen_paths.count(path)) {
			Logger::displayLog(
				Logger::e_log_level::ERROR, "Duplicate location path: " + path, "config");
			return ERROR;
		}
		seen_paths.insert(path);
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_checkDuplicateField(
	size_t bit_index, const std::string &field_name, std::bitset<8UL> &assigned_fields)
{
	if (assigned_fields.test(bit_index)) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Double field " + field_name, "config");
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

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_handleServerNameDirective(std::string &line, bool &extra_line)
{
	if (_checkDuplicateField(1, "server_name", _current_server_block._assigned_fields) == ERROR) return ERROR;
	if (_parseServerName(line) == ERROR) return ERROR;

	extra_line = false;
	_current_server_block._assigned_fields.set(1);
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

	if (_current_server_block._locations.has_value()) {
		for (auto &l : _current_server_block._locations.value()) {
			std::cout << l << std::endl;
		}
	}
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

	if (_current_server_block._cgi.has_value()) {
		for (auto &cgi : _current_server_block._cgi.value()) {
			std::cout << cgi << std::endl;
		}
	}
	_current_server_block._assigned_fields.set(7);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_dispatchServerDirective(std::ifstream &ifs, std::string &line, bool &extra_line)
{
	if (isValidHeaderFormat(line, "listen", false))
		return _handleListenDirective(ifs, line, extra_line);

	if (isValidHeaderFormat(line, "server_name", true))
		return _handleServerNameDirective(line, extra_line);

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

	Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field name: " + line, "config");
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
			getline(ifs, line);

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
			
			auto [it, inserted] = _server_blocks.emplace(_current_server_block._listen_data, _current_server_block);

			if (!inserted) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Duplicate (port,id) detected!", "config");
				return ERROR;
			}
		}
		else
		{
			Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field name: " + line, "config");
			return ERROR;
		}
	}
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::parse()
{
	std::ifstream ifs(_filename);
	if (ifs.bad()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Cannot access " + _filename, "config");
		return ERROR;
	}

	e_parse_result result = _parseAllServerBlocks(ifs);
	if (result == ERROR)
		return ERROR;

	return _validateServerBlocks();
}
