#ifndef LOGGER 
#define LOGGER 

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger
{
public:
	enum class e_log_level : char
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL,
		NONE
	};
	void displayLog(e_log_level level, const std::string& message, const std::string& module = "http");
	static constexpr e_log_level _max_log_lvl = e_log_level::DEBUG;

private:
	Logger() = delete;
	Logger(const Logger & other) = delete;
	Logger(const Logger && other) = delete;
	Logger & operator=(const Logger && other) = delete;
	~Logger() = delete;

	// ANSI color codes for terminal
	const std::string RESET  = "\033[0m";
	const std::string RED    = "\033[31m";
	const std::string YELLOW = "\033[33m";
	const std::string BLUE   = "\033[34m";
	const std::string CYAN   = "\033[36m";
	const std::string MAGENTA= "\033[35m";
};

#endif /* LOGGER */