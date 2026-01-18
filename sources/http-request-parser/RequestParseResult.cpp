#include "RequestParseResult.hpp"

RequestParseResult::RequestParseResult(uint status_code, const std::string& content, std::string_view method) :
	_status_code(status_code),
	_request_target(content),
	_method(method)
{
	std::string extension = std::string(std::filesystem::path(content).extension());
	extension = Trimmer::trim(extension, '\"');
	_content_type = HttpContentType::get_content_type_by_extension(extension);
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
	return _request_target;
}

std::string RequestParseResult::get_content_type() const
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
	_request_target = content;
}

void RequestParseResult::set_content_type(std::string content_type)
{
	_content_type = content_type;
}

void RequestParseResult::set_method(std::string_view method)
{
	_method = method;
}