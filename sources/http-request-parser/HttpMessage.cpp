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
	try
	{
		auto it = _headers.find(http::headers::CONTENT_LENGTH);
		if (it == _headers.end())
			return -1;
		ssize_t content_length = std::stoll(it->second);
		return content_length;
	}
	catch(const std::exception& e)
	{
		Log::error("Invalid content length header");
	}
	return -1;
}

void HttpMessage::set_header_value(std::string key, std::string new_value)
{
	_headers[key] = new_value;
}

void HttpMessage::remove_header_key(std::string key)
{
	_headers.erase(key);
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
	return _headers;
}

std::unordered_map<std::string, std::string> HttpMessage::move_headers()
{
	return std::move(_headers);
}

std::unordered_map<std::string, std::string> HttpMessage::get_headers() const
{
	return _headers;
}

void HttpMessage::append_body_value(std::string addition)
{
	_body.append(addition);
}

void HttpMessage::append_body_value(std::string addition, size_t bytes)
{
	_body.append(addition, bytes);
}

std::string &HttpMessage::get_body()
{
	return _body;
}
