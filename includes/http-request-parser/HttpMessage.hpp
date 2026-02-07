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
	HttpMessage(const HttpMessage &other) = default;
	HttpMessage(HttpMessage &&other) = default;
	HttpMessage & operator=( HttpMessage && ) noexcept = default;
	~HttpMessage();

	std::unordered_map<std::string, std::string> copy_headers();
	std::string									get_header_value(std::string key) const;
	void										set_headers(std::unordered_map<std::string, std::string> &&headers);
	size_t										get_header_count(std::string key) const;
	size_t										amount_of_headers() const;
	void										set_header_value(std::string key, std::string new_value);
	void										append_header_value(std::string key, std::string additional_value);
	ssize_t										get_content_length() const;

	void										append_body_value(std::string addition);
	std::string									get_body();
};

#endif /* REQUEST_PARSE_RESULT_HPP */
