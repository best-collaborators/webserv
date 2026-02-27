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
		server_name = line.substr(12);
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

	ErrorPage error_page;
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

ConfigurationFileParser::e_parse_result ConfigurationFileParser::_parseLocations(std::ifstream &ifs, std::string &line)
{
	std::cout << line << std::endl;
	std::string path = RegexMatcher::get_regex_value(line, HttpRegexPatterns::LOCATION_PATH());
	if (path.empty()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Location path is invalid: " + line, "config");
		return ERROR;
	}

	while (getline(ifs, line))
	{
		if (!_isStreamGood(ifs)) return ERROR;
		if (_isStreamFinished(ifs)) break;
		if (isEmptyLine(line)) continue;
		if (!_validateAndConsumeIndent(line, 3, '\t', false)) break;

		if (isValidHeaderFormat(line, "allowed_methods", true))
		{
			std::string methods_str = RegexMatcher::get_regex_value(line, HttpRegexPatterns::ALLOWED_METHODS());
			if (methods_str.empty()) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Allowed methods are invalid: " + line, "config");
				return ERROR;
			}
		}
		else if (isValidHeaderFormat(line, "allowed_methods", true))
		{

		}
		else if (isValidHeaderFormat(line, "default", true))
		{
			
		}
		else if (isValidHeaderFormat(line, "root", true))
		{
			
		}
		else if (isValidHeaderFormat(line, "autoindex", true))
		{
			
		}
	}

	Location location;
	location.path = path;
	Logger::displayLog(Logger::e_log_level::INFO, "Location path: " + path, "config");

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
	return (ifs.bad() && ifs.eof());
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
			while (getline(ifs, line))
			{
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
					_assigned_fields.set(0);
				}
				else if (isValidHeaderFormat(line, "server_name", true))
				{
					if (_assigned_fields.test(1)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format server_name", "config");
						return ERROR;
					}
					if (_parseServerName(line) == ERROR) return ERROR;
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
						if (!_validateAndConsumeIndent(line, 2, '\t', false)) break;
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
					_assigned_fields.set(3);
				}
				else if (isValidHeaderFormat(line, "root", true))
				{
					if (_assigned_fields.test(4)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format root", "config");
						return ERROR;
					}
					if (_parseRoot(line) == ERROR) return ERROR;
					_assigned_fields.set(4);
				}
				else if (isValidHeaderFormat(line, "index", true))
				{
					if (_assigned_fields.test(5)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format index", "config");
						return ERROR;
					}
					if (_parseIndex(line) == ERROR) return ERROR;
					_assigned_fields.set(5);
				}
				else if (isValidHeaderFormat(line, "locations", false))
				{
					if (_assigned_fields.test(6)) {
						Logger::displayLog(Logger::e_log_level::ERROR, "Double field format locations", "config");
						return ERROR;
					}

					while (getline(ifs, line))
					{
						if (!_isStreamGood(ifs)) return ERROR;
						if (_isStreamFinished(ifs)) break;
						if (isEmptyLine(line)) continue;
						if (!_validateAndConsumeIndent(line, 2, '\t', false)) break;
						if (_parseLocations(ifs, line) == ERROR) return ERROR;
					}
					_assigned_fields.set(6);
				}
				else if (isValidHeaderFormat(line, "cgi", false))
				{
					
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
		for (auto er : _data._error_pages)
		{
			// ! TO DO CHECK ONLY WHEN ROOT IS KNOWN
			std::ifstream ifs(_data._root + er.path);
			if (!ifs) {
				Logger::displayLog(Logger::e_log_level::ERROR, "Impossible to read: " + _data._root + er.path, "config");
				return ERROR;
			}
			Logger::displayLog(Logger::e_log_level::INFO, "Open " + _data._root + er.path, "config");
			ifs.close();
		}
	}
	return OK;
}