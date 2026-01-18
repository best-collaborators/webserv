#include "Clock.hpp"

std::string Clock::getCurrentTime() {
	std::time_t now = std::time(nullptr);
	std::tm* local = std::localtime(&now);

	char buffer[20];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local);
	return buffer;
}

long long Clock::getTimeMillis() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
	).count();
}
