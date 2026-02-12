#include "HttpMethod.hpp"

std::bitset<8> HttpMethod::_allowed_methods;

HttpMethod::HttpMethod()
{
	_allowed_methods.set();
	_allowed_methods.set(_code_to_uint(e_code::INVALID), 0);
}

HttpMethod::e_code HttpMethod::fromString(const std::string& method) {
	if (method == "OPTIONS") return e_code::OPTIONS;
	if (method == "GET") return e_code::GET;
	if (method == "POST") return e_code::POST;
	if (method == "DELETE") return e_code::DELETE;
	return e_code::INVALID;
}

std::string HttpMethod::toString(e_code code) {
	switch (code) {
		case e_code::OPTIONS: return "OPTIONS";
		case e_code::GET: return "GET";
		case e_code::POST: return "POST";
		case e_code::DELETE: return "DELETE";
		default:
			return "INVALID";
	}
}

uint HttpMethod::_code_to_uint(HttpMethod::e_code method)
{
	return static_cast<std::underlying_type_t<e_code>>(method);
}

bool HttpMethod::isAllowed(HttpMethod::e_code method)
{
	return _allowed_methods.test(_code_to_uint(method));
}

bool HttpMethod::isAllowed(std::string method)
{
	return _allowed_methods.test(_code_to_uint(fromString(method)));
}

void HttpMethod::setAllowedMethod(HttpMethod::e_code method)
{
	_allowed_methods.set(_code_to_uint(method), 1);
}

void HttpMethod::disableAllowedMethod(HttpMethod::e_code method)
{
	_allowed_methods.set(_code_to_uint(method), 0);
}

bool HttpMethod::hasBody(HttpMethod::e_code method)
{
	return method == e_code::POST;
}

bool HttpMethod::hasBody(std::string method)
{
	return method == "POST";
}

void HttpMethod::printAllowedMethods()
{
	std::string result;

	for (std::size_t i = 0; i < _allowed_methods.size(); ++i)
	{
		if (_allowed_methods.test(i))
		{
			result += toString(static_cast<HttpMethod::e_code>(i));
			result += ", ";
		}
	}

	if (!result.empty())
		result.erase(result.size() - 2);

	std::cout << result << std::endl;
}
