#include "ValidatorHelpers.hpp"

std::string ValidatorHelpers::cut_after_new_line(std::string &line)
{
	size_t pos = line.find("\r\n");
	if (pos == std::string::npos) return "";

	std::string temp_buffer = line.substr(0, pos);

	line.erase(0, pos + 2);
	return temp_buffer;
}
