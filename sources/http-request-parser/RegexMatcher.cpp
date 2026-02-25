#include "RegexMatcher.hpp"

std::string RegexMatcher::get_regex_value(std::string &line, std::regex regex_method)
{
	std::smatch matches;
	std::regex_search(line, matches, regex_method);
	if (matches.empty()) return "";
	std::string matched_string = matches.str(1);
	line = matches.suffix();
	return matched_string;
}

#include <iostream>
#include <iomanip>
#include "Logger.hpp"
std::string RegexMatcher::get_regex_value(std::string &line, std::regex regex_method, size_t match_number)
{
	std::smatch matches;

	if (!std::regex_search(line, matches, regex_method)) {
		Logger::displayLog(Logger::e_log_level::ERROR, "No match found: " + line, "config");
		return "";
	}
	if (match_number >= matches.size()) {
		Logger::displayLog(Logger::e_log_level::ERROR, "Invalid match group index: " + line, "config");
		return "";
	}

	if (matches.empty()) return "";
	std::string matched_string = matches.str(match_number);
	line = matches.suffix();
	return matched_string;
}