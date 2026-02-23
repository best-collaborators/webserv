#ifndef HTTP_METHOD_HPP
#define HTTP_METHOD_HPP

#include <iostream>
#include <bitset>

class HttpMethod {
public:
	enum class e_code : char {
		OPTIONS,
		GET,
		POST,
		DELETE,
		INVALID
	} t_code;

	static std::string	 toString(e_code code);
	static e_code 		 fromString(const std::string& method);

	static bool			 hasBody(HttpMethod::e_code method);
	static bool			 hasBody(std::string method);
	
private:
	HttpMethod & operator=( HttpMethod && ) noexcept = default;
	HttpMethod(const HttpMethod && other) = delete;
	HttpMethod(const HttpMethod & other) = delete;
	HttpMethod() = default;
	~HttpMethod() = default;
};

#endif /* HTTP_METHOD_HPP */
