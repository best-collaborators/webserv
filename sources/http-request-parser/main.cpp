#include "ResponseGenerator.hpp"
#include "RequestParser.hpp"

void print_http_request_values(std::unordered_map<std::string, std::string> httpRequestValues)
{
	for (auto values : httpRequestValues) {
		std::cout << "[" << values.first << "] " << "[" << values.second  << "] " << std::endl;
	}
}

int main()
{
	std::unordered_map<std::string, std::string> http_request_values;

	std::string root = "sources/http-parser";

	RequestParser requestParser(http_request_values);
	uint status_code = requestParser.get_status_code("");
	if (status_code == UINT8_MAX)
		return 1;

	std::cout << std::endl;
	print_http_request_values(http_request_values);

	ResponseGenerator responseGenerator(status_code, root + http_request_values["request-target"], "application/text", http_request_values["method"], true);
	responseGenerator.form_reponse();

	return status_code > 400 ? 1 : 0;
}
