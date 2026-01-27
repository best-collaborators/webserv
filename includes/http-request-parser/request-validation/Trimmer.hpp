#ifndef TRIMMER_HPP
#define TRIMMER_HPP

#include <string>
#include <algorithm>
#include <cctype>

class Trimmer
{
private:
	static std::string &ltrim(std::string &s);
	static std::string &rtrim(std::string &s);

	static std::string &ltrim(std::string &s, char delim);
	static std::string &rtrim(std::string &s, char delim);

public:
	static std::string &trim(std::string &s);
	static std::string &trim(std::string &s, char delim);
};

#endif