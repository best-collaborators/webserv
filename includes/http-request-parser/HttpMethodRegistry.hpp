#ifndef HTTP_METHOD_REGISTRY_HPP
#define HTTP_METHOD_REGISTRY_HPP

#include <iostream>
#include <bitset>
#include "HttpMethod.hpp"

class HttpMethodRegistry {
public:
	
	static std::bitset<8> _allowed_methods;

	static void			 initAllowedMethods();

	static bool			 isAllowed(HttpMethod::e_code request);
	static bool			 isAllowed(std::string method);
	static void			 setAllowedMethod(HttpMethod::e_code request);
	static void			 disableAllowedMethod(HttpMethod::e_code method);

	static void			 printAllowedMethods();

	HttpMethodRegistry & operator=( HttpMethodRegistry && ) noexcept = default;

	HttpMethodRegistry() = default;
	~HttpMethodRegistry() = default;
	HttpMethodRegistry(const HttpMethodRegistry & other) = default;

private:
	HttpMethodRegistry(const HttpMethodRegistry && other) = delete;
	static uint _code_to_uint(HttpMethod::e_code method);
};

#endif /* HTTP_METHOD_REGISTRY_HPP */
