#ifndef HTTP_METHOD_REGISTRY_HPP
#define HTTP_METHOD_REGISTRY_HPP

#include <iostream>
#include <bitset>
#include "HttpMethod.hpp"
#include "Logger.hpp"

class HttpMethodRegistry {
public:
	
	std::bitset<8> _allowed_methods;

	void			 initAllowedMethods();

	bool			 isAllowed(HttpMethod::e_code request);
	bool			 isAllowed(std::string method);
	void			 setAllowedMethod(HttpMethod::e_code request);
	void			 disableAllowedMethod(HttpMethod::e_code method);

	void			 printAllowedMethods() const;

	HttpMethodRegistry & operator=( const HttpMethodRegistry & ) noexcept;

	HttpMethodRegistry() = default;
	~HttpMethodRegistry() = default;
	HttpMethodRegistry(const HttpMethodRegistry & other) = default;
	
	std::string to_string() const;

private:
	HttpMethodRegistry(const HttpMethodRegistry && other) = delete;
	uint _code_to_uint(HttpMethod::e_code method);
};

#endif /* HTTP_METHOD_REGISTRY_HPP */
