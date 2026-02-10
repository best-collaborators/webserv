#include "PercentEncoder.hpp"

void PercentEncoder::percent_encoding(std::string &buffer)
{
	auto pos = std::find(buffer.begin(), buffer.end(), '%');
	while (pos != buffer.end() && (pos + 1) != buffer.end() && (pos + 2) != buffer.end())
	{
		size_t index = pos - buffer.begin();
		char hex[3];
		hex[0] = buffer[index + 1];
		hex[1] = buffer[index + 2];
		hex[2] = '\0';

		try
		{
			char char_encoded = std::stoi(hex, nullptr, 16);
			buffer.replace(index, 3, 1, char_encoded);
		}
		catch(const std::exception& e) { std::cout << "[http-parser] Not a percent encoding character" << std::endl; }
		pos = std::find(buffer.begin() + index + 1, buffer.end(), '%');
	}
}