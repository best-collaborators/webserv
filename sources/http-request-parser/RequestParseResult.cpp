#include "RequestParseResult.hpp"

RequestParseResult::RequestParseResult(uint status_code, const std::string& content, std::string_view method, std::string content_length_header) :
	_status_code(status_code),
	_request_target(content),
	_method(method)
{
	std::string extension = std::string(std::filesystem::path(content).extension());
	extension = Trimmer::trim(extension, '\"');
	_content_type = HttpContentType::get_content_type_by_extension(extension);

	if (!content_length_header.empty())
	{
		try
		{
			size_t pos;
			_content_length = std::stoull(content_length_header, &pos, 10);
			if (pos != content_length_header.size()) _content_length = -1;
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			_content_length = -1;
		}
		
	}
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

ssize_t RequestParseResult::get_content_length() const
{
	return _content_length;
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

void RequestParseResult::set_content_length(ssize_t content_length)
{
	_content_length = content_length;
}

void RequestParseResult::set_method(std::string_view method)
{
	_method = method;
}

// std::string RequestParseResult::get_header_value(const std::string& header_name) const
// {
// 	std::string lowercase_name = header_name;
// 	std::transform(lowercase_name.begin(), lowercase_name.end(), lowercase_name.begin(), [](unsigned char c){ return std::tolower(c); });

// 	auto it = _http_request_values.find(lowercase_name);
// 	if (it != _http_request_values.end()) {
// 		return it->second;
// 	}
// 	return "";
// }
