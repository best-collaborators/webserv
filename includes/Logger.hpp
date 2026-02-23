#ifndef LOGGER 
#define LOGGER 

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

#ifndef DEBUG_FLAG
	#define DEBUG_FLAG false
#endif

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
	static void displayLog(e_log_level level, const std::string& message, const std::string& module = "http");
	static constexpr e_log_level _max_log_lvl = e_log_level::DEBUG;

private:
	Logger() = delete;
	Logger(const Logger & other) = delete;
	Logger(const Logger && other) = delete;
	Logger & operator=(const Logger && other) = delete;
	~Logger() = delete;

	// ANSI color codes for terminal
	static const std::string RESET;
	static const std::string RED;
	static const std::string YELLOW;
	static const std::string BLUE;
	static const std::string CYAN;
	static const std::string MAGENTA;
};

#endif /* LOGGER */