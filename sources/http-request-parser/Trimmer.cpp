#include "Trimmer.hpp"

std::string &Trimmer::ltrim(std::string &s)
{
	auto is_space = [](int c) {return std::isspace(c);};
	auto not_space_iterator = std::find_if_not(s.begin(), s.end(), is_space);
	s.erase(s.begin(), not_space_iterator);
	return s;
}

std::string &Trimmer::rtrim(std::string &s)
{
	auto is_space = [](int c) {return std::isspace(c);};
	auto not_space_iterator = std::find_if_not(s.rbegin(), s.rend(), is_space);
	s.erase(not_space_iterator.base(), s.end());
	return s;
}

std::string &Trimmer::trim(std::string &s)
{
	s = ltrim(s);
	s = rtrim(s);
	return s;
}

std::string &Trimmer::ltrim(std::string &s, char delim)
{
	auto is_delim = [delim](int c) { return c == delim; };
	auto not_delim_iterator = std::find_if_not(s.begin(), s.end(), is_delim);
	s.erase(s.begin(), not_delim_iterator);
	return s;
}

std::string &Trimmer::rtrim(std::string &s, char delim)
{
	auto is_delim = [delim](int c) {return c == delim; };
	auto not_delim_iterator = std::find_if_not(s.rbegin(), s.rend(), is_delim);
	s.erase(not_delim_iterator.base(), s.end());
	return s;
}

std::string &Trimmer::trim(std::string &s, char delim)
{
	s = ltrim(s, delim);
	s = rtrim(s, delim);
	return s;
}