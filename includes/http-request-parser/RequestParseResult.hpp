#ifndef REQUEST_PARSE_RESULT_HPP
#define REQUEST_PARSE_RESULT_HPP

#include <iostream>
#include <filesystem>

#include "HttpContentType.hpp"
#include "Trimmer.hpp"

class RequestParseResult
{
private:
	uint 			  _status_code;
	std::string 	  _request_target;
	std::string		 _content_type;
	std::string_view  _method;

public:
	RequestParseResult(uint status_code, const std::string& request_target, std::string_view method);
	RequestParseResult();
	~RequestParseResult();

	uint get_status_code() const;
	std::string get_content() const;
	std::string get_content_type() const;
	std::string_view get_method() const;

	void set_status_code(uint status_code);
	void set_content(const std::string& request_target);
	void set_content_type(std::string content_type);
	void set_method(std::string_view method);
};

#endif /* REQUEST_PARSE_RESULT_HPP */
