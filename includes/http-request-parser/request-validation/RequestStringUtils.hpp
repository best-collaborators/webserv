#ifndef VALIDATOR_HELPERS_HPP
#define VALIDATOR_HELPERS_HPP

#include <string>
#include <algorithm>
#include <cctype>

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
	static std::string consume_next_line(std::string raw_bits);
};

#endif