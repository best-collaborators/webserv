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

template<Logger::e_log_level Level>
static inline void log(const std::string& msg, const std::string& mod = "http")
{
	Logger::displayLog(Level, msg, mod);
}

namespace Log {
	void debug   (const std::string& msg, const std::string& mod = "http");
	void info    (const std::string& msg, const std::string& mod = "http");
	void warning (const std::string& msg, const std::string& mod = "http");
	void error   (const std::string& msg, const std::string& mod = "http");
	void critical(const std::string& msg, const std::string& mod = "http");
}

#endif /* LOGGER */