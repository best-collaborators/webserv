#include "HttpMessage.hpp"

HttpMessage::HttpMessage(std::unordered_map<std::string, std::string> http_request_values)
: _headers(http_request_values) {}

HttpMessage::~HttpMessage() { }

std::string HttpMessage::get_header_value(std::string key) const
{
	std::string lowercase_name = key;
	std::transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), [](unsigned char c){ return std::tolower(c); });

	auto it = _headers.find(lowercase_name);
	if (it == _headers.end())
		return "";
	return it->second;
}

void HttpMessage::set_headers(std::unordered_map<std::string, std::string> &&headers)
{
	_headers = headers;
}

size_t	HttpMessage::get_header_count(std::string key) const
{
	return _headers.count(key);
}

size_t HttpMessage::amount_of_headers() const
{
	return _headers.size();
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

void HttpMessage::append_header_value(std::string key, std::string additional_value)
{
	std::string lowercase_name = key;
	std::transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), [](unsigned char c){ return std::tolower(c); });

	auto it = _headers.find(key);
	if (it == _headers.end())
		_headers[key] = additional_value;
	else
		_headers[key] += ", " + additional_value; 
}

std::unordered_map<std::string, std::string> HttpMessage::copy_headers()
{
	return std::move(_headers);
}

void HttpMessage::append_body_value(std::string addition)
{
	_body += addition;
}

std::string &HttpMessage::get_body()
{
	return _body;
}
