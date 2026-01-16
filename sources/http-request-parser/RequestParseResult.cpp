#include "RequestParseResult.hpp"

RequestParseResult::RequestParseResult(uint status_code, const std::string& content, std::string_view content_type, std::string_view method) :
	_status_code(status_code),
	_content(content),
	_content_type(content_type),
	_method(method)
{
}

RequestParseResult::RequestParseResult() {}

RequestParseResult::~RequestParseResult()
{
}

uint RequestParseResult::get_status_code() const
{
	return _status_code;
}

std::string RequestParseResult::get_content() const
{
	return _content;
}

std::string_view RequestParseResult::get_content_type() const
{
	return _content_type;
}

std::string_view RequestParseResult::get_method() const
{
	return _method;
}

void RequestParseResult::set_status_code(uint status_code)
{
	_status_code = status_code;
}

void RequestParseResult::set_content(const std::string& content)
{
	_content = content;
}

void RequestParseResult::set_content_type(std::string_view content_type)
{
	_content_type = content_type;
}

void RequestParseResult::set_method(std::string_view method)
{
	_method = method;
}