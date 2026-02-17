#ifndef PERCENT_ENCODER
#define PERCENT_ENCODER

#include <algorithm>
#include <iostream>

class PercentEncoder
{
private:

	PercentEncoder() = delete;
	PercentEncoder(const PercentEncoder && other) = delete;
	PercentEncoder(const PercentEncoder & other) = delete;
	PercentEncoder & operator=( PercentEncoder && ) noexcept = delete;
	~PercentEncoder() = delete;

public:
	static std::string percent_encoding(std::string &buffer);
};

#endif /* PERCENT_ENCODER */
