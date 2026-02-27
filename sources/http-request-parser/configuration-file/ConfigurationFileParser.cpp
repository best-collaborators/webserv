#include "ConfigurationFileParser.hpp"

ConfigurationFileParser::ConfigurationFileParser(std::string filename) : _filename(filename)
{
	_assigned_fields.reset();
}

bool isEmptyLine(std::string line)
{
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

#include <filesystem>
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
	_data._root = root_str;
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

	CGIPath cgi;
	cgi.path = path;
	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		Logger::displayLog(Logger::e_log_level::CRITICAL, line, "config");
		if (isValidHeaderFormat(line, "allowed_methods", true))
		{
			line.erase(0, 15);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			std::string methods_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::ALLOWED_METHODS(), 0);
			if (methods_str.empty()) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Allowed methods are invalid: " + line, "config");
				return ERROR;
			}

			std::cout << methods_str << std::endl; 
			if (!cgi.methods_registry.has_value())
				cgi.methods_registry.emplace();
			_updateAllowedMethods(methods_str, cgi.methods_registry.value());
		}
		else if (isValidHeaderFormat(line, "extension", true))
		{
			line.erase(0, 9);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			if (line != ".py" && line != ".js" && line != ".php") {
				Logger::displayLog(Logger::e_log_level::ERROR, "Extension is unsuported: " + line, "config");
				return ERROR;
			}

			cgi.extension = line;
		}
		else if (isValidHeaderFormat(line, "pass_to", true))
		{
			line.erase(0, 7);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			cgi.pass_to = line;
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

	Location location;
	location.path = path;
	while (getline(ifs, line))
	{
		
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (isValidHeaderFormat(line, "allowed_methods", true))
		{
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
		}
		else if (isValidHeaderFormat(line, "index", true))
		{
			line.erase(0, 5);
			Trimmer::trim(line);
			line.erase(0, 1);
			Trimmer::trim(line);

			location.default_file = line;
		}
		else if (isValidHeaderFormat(line, "root", true))
		{
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
			location.root = line;
		}
		else if (isValidHeaderFormat(line, "return", true))
		{
			line.erase(0, 5);
			Trimmer::trim(line);
			line.erase(0, 1);

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
		}
		else if (isValidHeaderFormat(line, "autoindex", true))
		{
			line.erase(0, 9);
			Trimmer::trim(line);
			line.erase(0, 1);

			Trimmer::trim(line);
			if (line != "on" && line != "off") {
				Logger::displayLog(Logger::e_log_level::ERROR, "Autoindex is invalid: " + line, "config");
				return ERROR;
			}

			location.autoindex = line == "on" ? true : false;
		}
		else return ERROR;
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
	std::string tabs(intend_level, c);
	if (line.size() < intend_level || line.compare(0, intend_level, tabs) != 0) {
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

				Logger::displayLog(Logger::e_log_level::DEBUG, line, "config");
				if (isValidHeaderFormat(line, "listen", false)) 
				{
					if (_assigned_fields.test(0)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format listen", "config");
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
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format server_name", "config");
						return ERROR;
					}
					if (_parseServerName(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(1);
				}
				else if (isValidHeaderFormat(line, "error_pages", false))
				{
					if (_assigned_fields.test(2)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format error_pages", "config");
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
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format max_body_size", "config");
						return ERROR;
					}

					if (_parseMaxBodySize(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(3);
				}
				else if (isValidHeaderFormat(line, "root", true))
				{
					if (_assigned_fields.test(4)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format root", "config");
						return ERROR;
					}
					if (_parseRoot(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(4);
				}
				else if (isValidHeaderFormat(line, "index", true))
				{
					if (_assigned_fields.test(5)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format index", "config");
						return ERROR;
					}
					if (_parseIndex(line) == ERROR) return ERROR;
					extra_line = false;
					_assigned_fields.set(5);
				}
				else if (isValidHeaderFormat(line, "locations", false))
				{
					if (_assigned_fields.test(6)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format locations", "config");
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

					for (size_t i = 0; i < _data._locations->size(); i++)
					{
						std::cout << _data._locations->at(i);
						std::cout << std::endl;
					}
					_assigned_fields.set(6);
				}
				else if (isValidHeaderFormat(line, "cgi", false))
				{
					if (_assigned_fields.test(7)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format cgi", "config");
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

					for (size_t i = 0; i < _data._cgi->size(); i++)
					{
						std::cout << _data._cgi->at(i);
						std::cout << std::endl;
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

		// TODO create a separate function for file permissions:
		// for (auto er : _data._error_pages)
		// {
		// 	// ! TO DO CHECK ONLY WHEN ROOT IS KNOWN
		// 	std::ifstream ifs(_data._root + er.path);
		// 	if (!ifs) {
		// 		Logger::displayLog(Logger::e_log_level::ERROR, "Impossible to read: " + _data._root + er.path, "config");
		// 		return ERROR;
		// 	}
		// 	Logger::displayLog(Logger::e_log_level::INFO, "Open " + _data._root + er.path, "config");
		// 	ifs.close();
		// }
	}
	return OK;
}