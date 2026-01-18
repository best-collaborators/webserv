#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <ctime>
#include <string>
#include <chrono>

class Clock
{
	private:
		/* data */
	public:
		Clock(/* args */) = delete;
		~Clock() = delete;

		static std::string getCurrentTime();
		static long long getTimeMillis();
};

#endif /*CLOCK_HPP*/