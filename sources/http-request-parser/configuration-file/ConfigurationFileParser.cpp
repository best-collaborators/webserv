#include "ConfigurationFileParser.hpp"

ConfigurationFileParser::ConfigurationFileParser(std::string filename) : _filename(filename)
{
	_assigned_fields.reset();
}

bool isEmptyLine(std::string line)
{
	if (line.empty()) return true;
	std::regex reg(HttpRegexPatterns::WHITESPACE());
	return std::regex_match(line, reg);
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseMaxBodySize(std::string &line)
{
	std::string body_size_str;
	size_t max_body_size = 0;
	try {
		body_size_str = line.substr(14);
		size_t pos = 0;
		max_body_size = std::stoll(body_size_str, &pos, 10);
		if (pos != body_size_str.size()) throw std::logic_error("Body size has wrong format");
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Body size is invalid: " + line, "config");
		return ERROR;
	}

	_data._max_body_size = max_body_size;
	Logger::displayLog(Logger::e_log_level::INFO, "Max body size: " + body_size_str, "config");
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseRoot(std::string &line)
{
	std::string root_str;
	try {
		root_str = line.substr(5);
		Trimmer::trim(root_str);
		if (!std::filesystem::is_directory(root_str)) throw std::logic_error("Root is invalid");
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Root is invalid: " + root_str, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Root: " + root_str, "config");
	_data._root = std::filesystem::weakly_canonical(root_str);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseIndex(std::string &line)
{
	std::string index_str;
	try {
		index_str = line.substr(6);
		Trimmer::trim(index_str);
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Index is invalid: " + index_str, "config");
		return ERROR;
	}

	//! TO DO CHECK ONLY WHEN ROOT IS KNOWN
	// std::ifstream ifs(_data._root + index_str);
	// if (!ifs) {
	// 	Logger::displayLog(Logger::e_log_level::ERROR, "Impossible to read: " + index_str, "config");
	// 	return ERROR;
	// }

	Logger::displayLog(Logger::e_log_level::INFO, "Index: " + index_str, "config");
	_data._index = index_str;
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseServerName(std::string &line)
{
	std::string server_name;
	try {
		server_name = line.substr(11);
		Trimmer::trim(server_name);
		server_name.erase(0, 1);
		Trimmer::trim(server_name);
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Server name is invalid: " + line, "config");
		return ERROR;
	}

	std::string server_name_validated = RegexMatcher::get_regex_value(server_name, HttpRegexPatterns::NON_WHITESPACE(), 1);

	if (server_name_validated.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Invalid server name", "config");
		return ERROR;
	}
	_data._server_name = server_name_validated;
	Logger::displayLog(Logger::e_log_level::INFO, "Server name: " + _data._server_name, "config");
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseErrorPages(std::string &line)
{
	std::string copy = line;
	std::string status_code = RegexMatcher::get_regex_value(copy, HttpRegexPatterns::GET_ERROR_PAGE(), 1);
	std::string er_page_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::GET_ERROR_PAGE(), 2);

	HttpPage error_page;
	try {
		error_page.path = er_page_str;
		error_page.status_code = std::stoi(status_code);
	}
	catch(const std::exception& e) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Error page status code is invalid: " + er_page_str, "config");
		return ERROR;
	}

	Logger::displayLog(Logger::e_log_level::INFO, "Status code: " + status_code, "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Error page: " + er_page_str, "config");
	_data._error_pages.push_back(error_page);
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

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseCGI(std::ifstream &ifs, std::string &line)
{
	std::string path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (path.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "CGI path is invalid: " + line, "config");
		return ERROR;
	}

	std::bitset<8> cgi_assigned_fields;

	CGIPath cgi;
	cgi.path = path;
	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (isValidHeaderFormat(line, "allowed_methods", true))
		{
			if (cgi_assigned_fields.test(0)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field allowed_methods", "config");
				return ERROR;
			}
			line.erase(0, 15);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			std::string methods_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::ALLOWED_METHODS(), 0);
			if (methods_str.empty()) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Allowed methods are invalid: " + line, "config");
				return ERROR;
			}

			if (!cgi.methods_registry.has_value())
				cgi.methods_registry.emplace();
			_updateAllowedMethods(methods_str, cgi.methods_registry.value());
			cgi_assigned_fields.set(0);
		}
		else if (isValidHeaderFormat(line, "extension", true))
		{
			if (cgi_assigned_fields.test(1)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field extension", "config");
				return ERROR;
			}
			line.erase(0, 9);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			if (line != ".py" && line != ".js" && line != ".php") {
				Logger::displayLog(Logger::e_log_level::ERROR, "Extension is unsuported: " + line, "config");
				return ERROR;
			}

			cgi.extension = line;
			cgi_assigned_fields.set(1);
		}
		else if (isValidHeaderFormat(line, "pass_to", true))
		{
			if (cgi_assigned_fields.test(2)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field pass_to", "config");
				return ERROR;
			}
			line.erase(0, 7);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			cgi.pass_to = line;
			cgi_assigned_fields.set(2);
		}
		else return ERROR;
	}

	if (!_data._cgi.has_value())
		_data._cgi.emplace();

	_data._cgi->push_back(cgi);
	return OK;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocations(std::ifstream &ifs, std::string &line)
{
	std::string path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (path.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Location path is invalid: " + line, "config");
		return ERROR;
	}

	std::bitset<8> location_assigned_fields;

	Location location;
	location.path = std::filesystem::weakly_canonical(path);
	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (isValidHeaderFormat(line, "allowed_methods", true))
		{
			if (location_assigned_fields.test(0)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field pass_to", "config");
				return ERROR;
			}
			line.erase(0, 15);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			std::string methods_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::ALLOWED_METHODS(), 0);
			if (methods_str.empty()) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Allowed methods are invalid: " + line, "config");
				return ERROR;
			}

			if (!location.methods_registry.has_value())
				location.methods_registry.emplace();
			_updateAllowedMethods(methods_str, location.methods_registry.value());
			location_assigned_fields.set(0);
		}
		else if (isValidHeaderFormat(line, "index", true))
		{
			if (location_assigned_fields.test(1)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field pass_to", "config");
				return ERROR;
			}
			line.erase(0, 5);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			location.default_file = line;
			location_assigned_fields.set(1);
		}
		else if (isValidHeaderFormat(line, "root", true))
		{
			if (location_assigned_fields.test(2)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field pass_to", "config");
				return ERROR;
			}
			try {
				line.erase(0, 4);
				Trimmer::trim(line);
				line.erase(0, 1);
				Trimmer::trim(line);

				//! Verify root after server root is known
				// if (!std::filesystem::is_directory(line)) throw std::logic_error("Location root is invalid");
			}
			catch(const std::exception& e) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Root is invalid: " + line, "config");
				return ERROR;
			}
			location.root = std::filesystem::weakly_canonical(line);
			location_assigned_fields.set(2);
		}
		else if (isValidHeaderFormat(line, "return", true))
		{
			if (location_assigned_fields.test(3)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field pass_to", "config");
				return ERROR;
			}
			line.erase(0, 5);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			std::string copy = line;
			std::regex reg("(3[0-9][0-9]) *( *\\S*) *$");
			std::string status_code = RegexMatcher::get_regex_value(copy, reg, 1);
			std::string path = RegexMatcher::get_regex_value(line, reg, 2);

			HttpPage page;
			try {
				page.path = path;
				page.status_code = std::stoi(status_code);
			}
			catch(const std::exception& e) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Error page status code is invalid: " + status_code, "config");
				return ERROR;
			}
			location.return_page = page;
			location_assigned_fields.set(3);
		}
		else if (isValidHeaderFormat(line, "autoindex", true))
		{
			if (location_assigned_fields.test(4)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Double field pass_to", "config");
				return ERROR;
			}
			line.erase(0, 9);
			Trimmer::trim(line);
			line.erase(0, 1);

			Trimmer::trim(line);
			if (line != "on" && line != "off") {
				Logger::displayLog(Logger::e_log_level::ERROR, "Autoindex is invalid: " + line, "config");
				return ERROR;
			}

			location.autoindex = line == "on" ? true : false;
			location_assigned_fields.set(4);
		}
		else
		{
			Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field: " + line, "config");
			return ERROR;
		}
	}

	if (!_data._locations.has_value())
		_data._locations.emplace();

	_data._locations->push_back(location);
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

	_data._listen_data.ip_address = ip_addr;

	try {
		_data._listen_data.port = std::stoi(port);
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
	if (line.size() < intend_level + 1 || line.compare(0, intend_level, checker) != 0 || line[intend_level] == '\t') {
		if (show_msg)
			Logger::displayLog(Logger::e_log_level::ERROR, "Invalid format: " + line, "config");
		return false;
	}
	line.erase(0, intend_level);
	return true;
}

ConfigurationFileParser::e_parse_result ConfigurationFileParser::parse()
{
	std::ifstream ifs(_filename);
	if (ifs.bad()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Cannot access " + _filename, "config");
		return e_parse_result::ERROR;
	}

	std::string line;
	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;

		if (isEmptyLine(line)) continue;
		Logger::displayLog(Logger::e_log_level::DEBUG, "Current field: " + line, "config");

		if (line == "server:")
		{
			bool extra_line = false;
			while (true)
			{
				if (!extra_line) {
					getline(ifs, line);
				}
				if (!_isStreamGood(ifs)) return ERROR;
				if (_isStreamFinished(ifs)) break;

				if (isEmptyLine(line)) continue;
				if (!_validateAndConsumeIndent(line, 1, '\t')) return ERROR;
				std::cout << line.size() << std::endl;

				Logger::displayLog(Logger::e_log_level::DEBUG, line, "config");
				if (isValidHeaderFormat(line, "listen", false)) 
				{
					if (_assigned_fields.test(0)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field listen", "config");
						return ERROR;
					}

					getline(ifs, line);
					if (!_isStreamGood(ifs)) return ERROR;
					if (_isStreamFinished(ifs)) break;
					if (!_validateAndConsumeIndent(line, 2, '\t')) return ERROR;
					if (_parseListen(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(0);
				}
				else if (isValidHeaderFormat(line, "server_name", true))
				{
					if (_assigned_fields.test(1)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field server_name", "config");
						return ERROR;
					}
					if (_parseServerName(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(1);
				}
				else if (isValidHeaderFormat(line, "error_pages", false))
				{
					if (_assigned_fields.test(2)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field error_pages", "config");
						return ERROR;
					}

					while (getline(ifs, line))
					{
						if (!_isStreamGood(ifs)) return ERROR;
						if (_isStreamFinished(ifs)) break;
						if (isEmptyLine(line)) continue;
						if (!_validateAndConsumeIndent(line, 2, '\t', false)) {
							extra_line = true;
							break;
						}
						if (_parseErrorPages(line) == ERROR) return ERROR;
					}
					_assigned_fields.set(2);
				}
				else if (isValidHeaderFormat(line, "max_body_size", true))
				{

					if (_assigned_fields.test(3)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field max_body_size", "config");
						return ERROR;
					}

					if (_parseMaxBodySize(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(3);
				}
				else if (isValidHeaderFormat(line, "root", true))
				{
					if (_assigned_fields.test(4)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field root", "config");
						return ERROR;
					}
					if (_parseRoot(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(4);
				}
				else if (isValidHeaderFormat(line, "index", true))
				{
					if (_assigned_fields.test(5)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field index", "config");
						return ERROR;
					}
					if (_parseIndex(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(5);
				}
				else if (isValidHeaderFormat(line, "locations", false))
				{
					if (_assigned_fields.test(6)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field locations", "config");
						return ERROR;
					}

					getline(ifs, line);
					while (true)
					{
						if (!_isStreamGood(ifs)) return ERROR;
						if (_isStreamFinished(ifs)) break;
						if (isEmptyLine(line)) {
							getline(ifs, line);
							continue;
						}
						if (!_validateAndConsumeIndent(line, 2, '\t', false)){
							extra_line = true;
							break;
						}
						if (_parseLocations(ifs, line) == ERROR) return ERROR;
					}

					if (_data._locations.has_value())
					{
						for (auto &l : _data._locations.value())
						{
							std::cout << l;
							std::cout << std::endl;
						}
					}
					_assigned_fields.set(6);
				}
				else if (isValidHeaderFormat(line, "cgi", false))
				{
					if (_assigned_fields.test(7)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field cgi", "config");
						return ERROR;
					}

					getline(ifs, line);
					while (true)
					{
						if (!_isStreamGood(ifs)) return ERROR;
						if (_isStreamFinished(ifs)) break;
						if (isEmptyLine(line)) {
							getline(ifs, line);
							continue;
						}
						if (!_validateAndConsumeIndent(line, 2, '\t', false)){
							extra_line = true;
							break;
						}
						if (_parseCGI(ifs, line) == ERROR) return ERROR;
					}

					if (_data._cgi.has_value())
					{
						for (auto &cgi : _data._cgi.value())
						{
							std::cout << cgi;
							std::cout << std::endl;
						}
					}
					_assigned_fields.set(7);
				}
				else
				{
					Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field name: " + line, "config");
					return ERROR;
				}
			}
		}
		else
		{
			Logger::displayLog(Logger::e_log_level::ERROR, "Invalid field name: " + line, "config");
			return ERROR;
		}

		for (size_t i = 0; i < _assigned_fields.size(); i++)
		{
			if (!_assigned_fields.test(i) && (i == 0 || i == 1 || i == 3 || i == 4) ) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Missing field", "config");
				return ERROR;
			}
		}

		for (auto &er : _data._error_pages)
		{
			// ! TO DO CHECK ONLY WHEN ROOT IS KNOWN
			std::ifstream ifs(_data._root.string() + er.path);
			if (!ifs) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Impossible to read: " + _data._root.string() + er.path, "config");
				return ERROR;
			}
			Logger::displayLog(Logger::e_log_level::INFO, "Open " + _data._root.string() + er.path, "config");
			ifs.close();
		}

		{
			std::ifstream ifs(_data._root.string() + _data._index->data());
			if (!ifs) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Impossible to read: " + _data._root.string() + _data._index->data(), "config");
				return ERROR;
			}
			Logger::displayLog(Logger::e_log_level::INFO, "Open " + _data._root.string() + _data._index->data(), "config");
			ifs.close();
		}

		for (auto &l : _data._locations.value())
		{
			if (l.root.empty()) l.root = _data._root.string();
			else l.root = _data._root.string() + l.root.string();

			if (!std::filesystem::is_directory(l.root)) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Is not a dir: " + l.root.string(), "config");
				return ERROR;
			}

			std::filesystem::path full = l.root.string() + "/" + l.default_file;
			full = std::filesystem::weakly_canonical(full);

			if (full.string().find(l.root) == std::string::npos)
			{
				Logger::displayLog(Logger::e_log_level::CRITICAL, l.root, "config");
				Logger::displayLog(Logger::e_log_level::CRITICAL, l.default_file, "config");
				Logger::displayLog(Logger::e_log_level::CRITICAL, "Wrong file path: " + full.string(), "config");
				return ERROR;
			}

			if (!l.default_file.empty())
			{
				std::ifstream ifs(l.root.string() + "/" + l.default_file);
				if (!ifs) {
					Logger::displayLog(Logger::e_log_level::ERROR, "Impossible to read: " + l.root.string()  + "/" + l.default_file, "config");
					return ERROR;
				}
				Logger::displayLog(Logger::e_log_level::INFO, "Open " + _data._root.string() + _data._index->data(), "config");
				ifs.close();
			}
		}
		
	}
	return OK;
}
