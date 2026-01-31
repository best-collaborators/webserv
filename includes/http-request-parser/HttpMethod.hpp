#ifndef HTTP_METHOD_HPP
#define HTTP_METHOD_HPP

#include <string>

class HttpMethod {
public:
	enum class e_code : short {
		OPTIONS,
		GET,
		POST,
		DELETE,
		INVALID
	} t_code;

	e_code HttpMethod::fromString(const std::string& method);
	std::string HttpMethod::toString(e_code code);
};

#endif /* HTTP_METHOD_HPP */
