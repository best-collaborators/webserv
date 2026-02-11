#include "RequestStringUtils.hpp"

std::string RequestStringUtils::cut_after_new_line(std::string &line)
{
	size_t pos = line.find("\r\n");
	if (pos == std::string::npos) return "";

	std::string temp_buffer = line.substr(0, pos);

	line.erase(0, pos + 2);
	return temp_buffer;
}

std::string RequestStringUtils::transform_to_lower(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
	return str;
}

std::string RequestStringUtils::consume_next_line(std::string raw_bits) {
    std::string temp = RequestStringUtils::cut_after_new_line(raw_bits);
    if (temp.empty() && !raw_bits.empty()) {
        temp = std::move(raw_bits);
        raw_bits.clear();
    }
    return temp;
}