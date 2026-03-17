#ifndef HTTP_STATUS_HPP
#define HTTP_STATUS_HPP

#include <string>

class HttpStatus {
public:
	enum class e_code : short {
		OK = 200,
		CREATED = 201,
		NO_CONTENT = 204,
		MOVED_PERMANENTLY = 301,
		BAD_REQUEST = 400,
		NOT_FOUND = 404,
		METHOD_NOT_ALLOWED = 405,
		LENGTH_REQUIRED = 411,
		CONTENT_TOO_LARGE = 413,
		URI_TOO_LONG = 414,
		REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
		GATEWAY_TIMEOUT = 504,
		SERVICE_UNAVAILABLE = 503,
		HTTP_VERSION_NOT_SUPPORTED = 505,
		INTERNAL_SERVER_ERROR = 500,
		UNKNOWN = 0
	} t_code;

	static std::string	get_status_code_name(e_code s);
	static e_code		code_from_number(uint code);
	static uint			number_from_code(e_code code);
	static bool			is_bad(e_code code);
	static bool			is_good(e_code code);
	static bool			is_redirect(e_code code);
	static std::string	get_message(e_code code);
};

std::ostream& operator<<(std::ostream& os, HttpStatus::e_code code);

#endif /* HTTP_STATUS_HPP */
