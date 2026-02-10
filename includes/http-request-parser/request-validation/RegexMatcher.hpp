#ifndef REGEX_MATCHER
#define REGEX_MATCHER

#include <string>
#include <regex>

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
};

#endif /* REGEX_MATCHER */