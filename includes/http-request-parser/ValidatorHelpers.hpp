#ifndef VALIDATOR_HELPERS_HPP
#define VALIDATOR_HELPERS_HPP

#include <string>
#include <algorithm>
#include <cctype>

class ValidatorHelpers
{
	public:
		ValidatorHelpers() = delete;
		~ValidatorHelpers() = delete;
		static std::string cut_after_new_line(std::string &line);

};

#endif