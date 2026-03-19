#include "PercentEncoder.hpp"

std::string PercentEncoder::percent_encoding(std::string &buffer)
{
	std::string copy = buffer;

	auto pos = std::find(copy.begin(), copy.end(), '%');
	while (pos != copy.end() && (pos + 1) != copy.end() && (pos + 2) != copy.end())
	{
		size_t index = pos - copy.begin();
		char hex[3];
		hex[0] = copy[index + 1];
		hex[1] = copy[index + 2];
		hex[2] = '\0';

		try
		{
			char char_encoded = std::stoi(hex, nullptr, 16);
			copy.replace(index, 3, 1, char_encoded);
		}
		catch(const std::exception& e) {
			Log::error("Not a percent encoding character", "http-parser");
		}
		pos = std::find(copy.begin() + index + 1, copy.end(), '%');
	}
	return copy;
}