#include "Logger.hpp"

void Logger::displayLog(e_log_level level, const std::string& message, const std::string& module)
{
	if (level < _max_log_lvl) return;

	std::string levelStr;
	std::string color;

	switch (level) {
		case e_log_level::DEBUG:    levelStr = "DEBUG";    color = BLUE; break;
		case e_log_level::INFO:     levelStr = "INFO";     color = CYAN; break;
		case e_log_level::WARNING:  levelStr = "WARNING";  color = YELLOW; break;
		case e_log_level::ERROR:    levelStr = "ERROR";    color = RED; break;
		case e_log_level::CRITICAL: levelStr = "CRITICAL"; color = MAGENTA; break;
		default:                     levelStr = "UNKNOWN"; color = RESET; break;
	}

	// Get current time
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	auto localTime = std::localtime(&time);

	// Print log
	std::cout << "[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "] "
				<< color << "[" << levelStr << "]" << RESET
				<< " [" << module << "] "
				<< message
				<< std::endl;
}