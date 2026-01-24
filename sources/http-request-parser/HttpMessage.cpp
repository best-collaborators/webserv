#include "HttpMessage.hpp"

HttpMessage::HttpMessage(std::unordered_map<std::string, std::string> http_request_values)
: _headers(http_request_values) {}

HttpMessage::~HttpMessage() { }

const std::string* HttpMessage::get_header_value(std::string key) const
{
	std::string lowercase_name = key;
	std::transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), [](unsigned char c){ return std::tolower(c); });

	auto it = _headers.find(key);
	if (it == _headers.end())
		return nullptr;
	return &(it->second);
}

ssize_t HttpMessage::get_content_length() const
{
	auto it = _headers.find("content-length");
	if (it == _headers.end())
		return -1;
	return std::stoll(it->second);
}

void HttpMessage::set_header_value(std::string key, std::string new_value)
{
	_headers[key] = new_value;
}

std::unordered_map<std::string, std::string> HttpMessage::copy_headers()
{
	return std::move(_headers);
}