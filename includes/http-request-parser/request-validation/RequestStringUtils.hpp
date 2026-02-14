#ifndef VALIDATOR_HELPERS_HPP
#define VALIDATOR_HELPERS_HPP

#include <iostream>
#include <algorithm>
#include <cctype>

#include "RegexMatcher.hpp"
#include "Trimmer.hpp"

#include <iomanip>

class RequestStringUtils
{
private:
	RequestStringUtils() = delete;
	RequestStringUtils(const RequestStringUtils &other) = delete;
	RequestStringUtils(RequestStringUtils &&other) = delete;
	RequestStringUtils & operator=( RequestStringUtils && ) noexcept = delete;
	~RequestStringUtils() = delete;

public:
	static std::string cut_after_new_line(std::string &line);
	static std::string transform_to_lower(std::string &str);
	static std::string consume_next_line(std::string &raw_bits);

	static bool tryExtractHeaderField(
		std::string &value,
		std::string &buffer,
		std::regex regex_method,
		std::string errmsg
	);
};

#endif