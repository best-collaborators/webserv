#ifndef REGEX_MATCHER
#define REGEX_MATCHER

#include <string>
#include <regex>
#include "Logger.hpp"

class RegexMatcher
{
private:
	RegexMatcher() = delete;
	RegexMatcher(const RegexMatcher &other) = delete;
	RegexMatcher(RegexMatcher &&other) = delete;
	RegexMatcher & operator=( RegexMatcher && ) noexcept = delete;
	~RegexMatcher() = delete;

public:
	static std::string get_regex_value(std::string &line, std::regex regex_method);
	static std::string get_regex_value(std::string &line, std::regex regex_method, size_t match_number, bool show_msg = true);
};

#endif /* REGEX_MATCHER */