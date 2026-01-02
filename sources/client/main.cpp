#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8000
#define IPADDR "127.0.0.1"

int _socket()
{
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}
	return sockfd;
}

sockaddr_in _getaddrinfo()
{
	struct sockaddr_in _addrinfo {};
	_addrinfo.sin_family = AF_INET;
	_addrinfo.sin_port = htons(PORT);

	inet_pton(AF_INET, IPADDR, &_addrinfo.sin_addr);
	return _addrinfo;
}

int _connect(int sockfd, struct sockaddr_in _addrinfo)
{
	if (connect(sockfd, (sockaddr *)&_addrinfo, sizeof(_addrinfo)) < 0) {
		perror("connect"); return 1;
	}
	return 0;
}

void create_get_response(std::string filename, std::string &buffer)
{
	std::string temp;
	std::ifstream ifs("tests/" + filename);
	if (ifs.bad()) return ;

	while (getline(ifs, temp)) {
		buffer += temp + "\r\n";
	}
	buffer += "\r\n";
}

void create_post_response(std::string &buffer)
{
	std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
	std::ifstream file("tests/profile.jpg", std::ios::binary);
	if (!file) { std::cerr << "Failed to open profile.jpg\n"; return ; }
	std::ostringstream file_data;
	file_data << file.rdbuf();
	std::string file_content = file_data.str();

	std::ostringstream body;
	body << "--" << boundary << "\r\n"
		 << "Content-Disposition: form-data; name=\"task\"\r\n\r\n"
		 << "john_doe\r\n"
		 << "--" << boundary << "\r\n"
		 << "Content-Disposition: form-data; name=\"profile_picture\"; filename=\"profile.jpg\"\r\n"
		 << "Content-Type: image/jpeg\r\n\r\n"
		 << file_content << "\r\n"
		 << "--" << boundary << "\r\n"
		 << "Content-Disposition: form-data; name=\"metadata\"\r\n"
		 << "Content-Type: application/json\r\n\r\n"
		 << "{\"age\":30,\"location\":\"New York\"}\r\n"
		 << "--" << boundary << "--\r\n";

	std::string body_str = body.str();

	std::ostringstream request;
	request << "POST /list/new HTTP/1.1\r\n"
		<< "Host: localhost:8000\r\n"
		<< "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n"
		<< "Content-Length: " << body_str.size() << "\r\n\r\n"
		<< "Connection: close\r\n\r\n"
		<< body_str;

	buffer = request.str();
}

void create_post_response(std::string &buffer)
{
	std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

	std::ostringstream body;
	body
		<< "--" << boundary << "\r\n"
		<< "Content-Disposition: form-data; name=\"task\"\r\n"
		<< "\r\n"
		<< "fffffff\r\n"
		<< "--" << boundary << "--\r\n";

	std::string body_str = body.str();

	std::ostringstream request;
	request
		<< "POST /list/new HTTP/1.1\r\n"
		<< "Host: localhost:8000\r\n"
		<< "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n"
		<< "Content-Length: " << body_str.size() << "\r\n"
		<< "\r\n"
		<< body_str;

	buffer = request.str();
}

int main()
{
	int sockfd = _socket();
	if (sockfd < 0) return 1;

	struct sockaddr_in _addrinfo = _getaddrinfo();
	if (_connect(sockfd, _addrinfo)) return 1;

	std::string message;
	create_post_response(message);
	if (message == "") {
		perror("create post");
		return 1;
	}

	// std::string message;
	// create_post_response2(message);
	// if (message == "") {
	// 	perror("create post");
	// 	return 1;
	// }

	// std::string message;
	// get_msg("get_connection_close.txt", message);

	std::cout << message << "---" << std::endl;
	if (send(sockfd, message.c_str(), message.size(), 0) != (ssize_t)message.size()) {
		perror("send"); return 1;
	}

	char recv_buffer[1024];
	std::string buffer;
	ssize_t n;
	while ((n = recv(sockfd, recv_buffer, sizeof(recv_buffer) - 1, 0)) > 0)
	{
		recv_buffer[n] = '\0';
		std::cout << recv_buffer;
	}
	if (n < 0) perror("recv");

	return 0;
}