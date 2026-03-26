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

std::string RegexMatcher::get_regex_value(std::string &line, std::regex regex_method, size_t match_number, bool show_msg)
{
	std::smatch matches;
	std::regex_search(line, matches, regex_method);
	if (matches.empty()) {
		if (show_msg)
			Logger::displayLog(Logger::e_log_level::DEBUG, "No regex match found", "config");
		return "";
	}

	if (match_number >= matches.size()) {
		if (show_msg)
			Logger::displayLog(Logger::e_log_level::DEBUG, "Invalid match group index: " + line, "config");
		return "";
	}

	if (matches.empty()) return "";
	std::string matched_string = matches.str(match_number);
	line = matches.suffix();
	return matched_string;
}