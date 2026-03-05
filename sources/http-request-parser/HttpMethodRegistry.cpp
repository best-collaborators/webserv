#include "HttpMethodRegistry.hpp"

void HttpMethodRegistry::initAllowedMethods()
{
	_allowed_methods.set();
	_allowed_methods.set(_code_to_uint(HttpMethod::e_code::INVALID), 0);
}

uint HttpMethodRegistry::_code_to_uint(HttpMethod::e_code method)
{
	return static_cast<std::underlying_type_t<HttpMethod::e_code>>(method);
}

bool HttpMethodRegistry::isAllowed(HttpMethod::e_code method)
{
	return _allowed_methods.test(_code_to_uint(method));
}

bool HttpMethodRegistry::isAllowed(std::string method)
{
	return _allowed_methods.test(_code_to_uint(HttpMethod::fromString(method)));
}

void HttpMethodRegistry::setAllowedMethod(HttpMethod::e_code method)
{
	if (method == HttpMethod::e_code::INVALID) return ;
	_allowed_methods.set(_code_to_uint(method), 1);
}

void HttpMethodRegistry::disableAllowedMethod(HttpMethod::e_code method)
{
	_allowed_methods.set(_code_to_uint(method), 0);
}

void HttpMethodRegistry::printAllowedMethods() const
{
	std::string result;

	for (std::size_t i = 0; i < _allowed_methods.size(); ++i)
	{
		if (_allowed_methods.test(i))
		{
			result += HttpMethod::toString(static_cast<HttpMethod::e_code>(i));
			result += ", ";
		}
	}

	if (!result.empty())
		result.erase(result.size() - 2);

	Logger::displayLog(Logger::e_log_level::INFO, "List of allowed methods: " + result, "method-registry");
}

std::string HttpMethodRegistry::to_string() const
{
	std::string result;

	for (std::size_t i = 0; i < _allowed_methods.size(); ++i)
	{
		if (_allowed_methods.test(i))
		{
			result += HttpMethod::toString(static_cast<HttpMethod::e_code>(i));
			result += ", ";
		}
	}

	if (!result.empty())
		result.erase(result.size() - 2);

	return result;
}
