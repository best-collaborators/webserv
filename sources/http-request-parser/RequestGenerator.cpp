#include "RequestGenerator.hpp"

void RequestGenerator::create_body_for_request(std::string &body, std::string &file_to_send, std::string &boundary)
{
	std::fstream file(file_to_send, std::ios::binary | std::ios::in);
	if (!file) { std::cerr << "Failed to open profile.jpg\n"; return ; }

	std::string file_content;
	while (true)
	{
		char content[100];
		file.read(content, 100);
		std::streamsize gcount = file.gcount();
		file_content.append(content, gcount);
		if (file.eof()) break;
	}

	// Append text parts
	body.append("--" + boundary + "\r\n");
	body.append("Content-Disposition: form-data; name=\"task\"\r\n\r\n");
	body.append("john_doe\r\n");

	// Append image part (binary-safe)
	body.append("--" + boundary + "\r\n");
	body.append("Content-Disposition: form-data; name=\"profile_picture\"; filename=\"profile.jpg\"\r\n");
	body.append("Content-Type: image/jpeg\r\n\r\n");
	body.append(file_content.data(), file_content.size());
	body.append("\r\n");

	// Append JSON part
	body.append("--" + boundary + "\r\n");
	body.append("Content-Disposition: form-data; name=\"metadata\"; filename=\"jsonfile\"\r\n");
	body.append("Content-Type: application/json\r\n\r\n");
	body.append("{\"age\":30,\"location\":\"New York\"}\r\n");

	// Closing boundary
	body.append("--" + boundary + "--\r\n");
}

void RequestGenerator::create_post_request(std::string &request)
{
	std::string file_to_send = "./tests/cat.jpg";
	std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

	std::string body;
	create_body_for_request(body, file_to_send, boundary);

	request.append("POST /list/new HTTP/1.1\r\n");
	request.append("Host: localhost:8000\r\n");
	request.append("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
	request.append("Content-Length: " + std::to_string(body.size()) + "\r\n");
	request.append("Connection: close\r\n\r\n");
	request.append(body.data(), body.size());
}
