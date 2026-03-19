#include "HttpStatus.hpp"

std::string HttpStatus::get_status_code_name(e_code s)
{
	switch (s)
	{
		case e_code::OK:						  return "OK";
		case e_code::BAD_REQUEST:				  return "Bad Request";
		case e_code::CREATED:					  return "Created";
		case e_code::NO_CONTENT:				  return "No Content";
		case e_code::MOVED_PERMANENTLY:			  return "Moved Permanently";
		case e_code::NOT_FOUND: 				  return "Not Found";
		case e_code::FORBIDDEN: 				  return "Forbidden";
		case e_code::METHOD_NOT_ALLOWED: 		  return "Method Not Allowed";
		case e_code::LENGTH_REQUIRED: 			  return "Length Required";
		case e_code::CONTENT_TOO_LARGE: 		  return "Content Too Large";
		case e_code::URI_TOO_LONG: 				  return "Uri Too Long";
		case e_code::HTTP_VERSION_NOT_SUPPORTED:  return "Http Version Not Supported";
		case e_code::GATEWAY_TIMEOUT: 			  return "Gateway Timeout";
		case e_code::SERVICE_UNAVAILABLE: 		  return "Service Unavailable";
		case e_code::INTERNAL_SERVER_ERROR: 	  return "Internal Server Error";
		default:
			return "";
	}
}

std::string HttpStatus::get_message(e_code code) {
	switch (code) {
		// 2xx Success
		case e_code::OK:
		case e_code::CREATED:
		case e_code::NO_CONTENT:
			return "Request successful";

		// 3xx Redirection
		case e_code::MOVED_PERMANENTLY:
			return "Resource moved";

		// 4xx Client errors
		case e_code::BAD_REQUEST:
			return "Invalid request";

		case e_code::NOT_FOUND:
			return "Resource not found";

		case e_code::METHOD_NOT_ALLOWED:
			return "Method not allowed";

		case e_code::LENGTH_REQUIRED:
		case e_code::CONTENT_TOO_LARGE:
		case e_code::URI_TOO_LONG:
		case e_code::REQUEST_HEADER_FIELDS_TOO_LARGE:
			return "Request too large or malformed";

		// 5xx Server errors
		case e_code::INTERNAL_SERVER_ERROR:
			return "Server error";

		case e_code::SERVICE_UNAVAILABLE:
			return "Service unavailable";

		case e_code::GATEWAY_TIMEOUT:
			return "Gateway timeout";

		case e_code::HTTP_VERSION_NOT_SUPPORTED:
			return "Protocol not supported";

		default:
			return "Unknown status";
	}
}

std::ostream& operator<<(std::ostream& os, HttpStatus::e_code code)
{
	return os << (std::to_string(static_cast<int>(code)) + " " + HttpStatus::get_status_code_name(code));
}

uint HttpStatus::number_from_code(e_code code)
{
	return static_cast<uint>(code);
}

HttpStatus::e_code HttpStatus::code_from_number(uint code)
{
	return e_code(code);
}

bool HttpStatus::is_bad(e_code code)
{
	return static_cast<int>(code) >= static_cast<std::underlying_type_t<e_code>>(e_code::BAD_REQUEST);
}

bool HttpStatus::is_redirect(e_code code)
{
	return static_cast<int>(code) >= static_cast<std::underlying_type_t<e_code>>(e_code::MOVED_PERMANENTLY)
	&& static_cast<int>(code) < static_cast<std::underlying_type_t<e_code>>(e_code::BAD_REQUEST);;
}

bool HttpStatus::is_good(e_code code)
{
	return static_cast<int>(code) > 0 &&
	static_cast<int>(code) < static_cast<std::underlying_type_t<e_code>>(e_code::MOVED_PERMANENTLY);
}