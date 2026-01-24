#ifndef REQUEST_PARSE_RESULT_HPP
#define REQUEST_PARSE_RESULT_HPP

#include <iostream>
#include <filesystem>
#include <unordered_map>

#include "HttpContentType.hpp"
#include "Trimmer.hpp"

class  HttpMessage
{
protected:
	std::unordered_map<std::string, std::string> _headers;
	std::string _body;
public:
	HttpMessage(std::unordered_map<std::string, std::string> _http_request_values);
	HttpMessage() = default;
	~HttpMessage();

	std::unordered_map<std::string, std::string> copy_headers();
	const std::string* get_header_value(std::string key) const;
	void set_header_value(std::string key, std::string new_value);
	ssize_t get_content_length() const;
};

#endif /* REQUEST_PARSE_RESULT_HPP */
