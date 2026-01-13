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

	std::string request = 
		"POST /upload HTTP/1.1\r\n"
		"Host: kvalerii\r\n"
		"Content-length: 59\r\n"
		"Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"john_doe\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"profile_picture\"; filename=\"1.jpg\"\r\n"
		"Content-Type: image/jpeg\r\n"
		"\r\n"
		"[Binary data of the JPEG file]\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"metadata\"\r\n"
		"Content-Type: application/json\r\n"
		"\r\n"
		"{\"age\": 30, \"location\": \"New York\"}\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n\r\n";

	RequestParser requestParser(http_request_values, request);
	uint status_code = requestParser.get_status_code();
	if (status_code == UINT8_MAX)
		return 1;

	print_http_request_values(http_request_values);

	ResponseGenerator responseGenerator(status_code, root + http_request_values["request-target"], "application/text", http_request_values["method"], true);
	responseGenerator.form_reponse();

	return status_code > 400 ? 1 : 0;
}
