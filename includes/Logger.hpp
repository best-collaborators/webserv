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
	inline void debug   (const std::string& msg, const std::string& mod = "http") { Logger::displayLog(Logger::e_log_level::DEBUG,    msg, mod); }
	inline void info    (const std::string& msg, const std::string& mod = "http") { Logger::displayLog(Logger::e_log_level::INFO,     msg, mod); }
	inline void warning (const std::string& msg, const std::string& mod = "http") { Logger::displayLog(Logger::e_log_level::WARNING,  msg, mod); }
	inline void error   (const std::string& msg, const std::string& mod = "http") { Logger::displayLog(Logger::e_log_level::ERROR,    msg, mod); }
	inline void critical(const std::string& msg, const std::string& mod = "http") { Logger::displayLog(Logger::e_log_level::CRITICAL, msg, mod); }
}

#endif /* LOGGER */