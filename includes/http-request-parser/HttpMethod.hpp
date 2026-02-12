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

	HttpMethod();
	~HttpMethod() = default;

	static std::bitset<8> _allowed_methods;

	static e_code 		 fromString(const std::string& method);
	static std::string	 toString(e_code code);

	static bool			 hasBody(HttpMethod::e_code method);
	static bool			 hasBody(std::string method);

	static bool			 isAllowed(HttpMethod::e_code request);
	static bool			 isAllowed(std::string method);
	static void			 setAllowedMethod(HttpMethod::e_code request);
	static void			 disableAllowedMethod(HttpMethod::e_code method);

	static void			 printAllowedMethods();

private:
	HttpMethod(const HttpMethod && other) = delete;
	HttpMethod(const HttpMethod & other) = delete;

	static uint _code_to_uint(HttpMethod::e_code method);
};

#endif /* HTTP_METHOD_HPP */
