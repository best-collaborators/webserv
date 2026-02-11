#include "HttpStatus.hpp"

std::string HttpStatus::get_status_code_name(e_code s)
{
	switch (s)
	{
		case e_code::OK:						  return "OK";
		case e_code::BAD_REQUEST:				  return "Bad Request";
		case e_code::CREATED:					  return "Created";
		case e_code::NO_CONTENT:				  return "No Content";
		case e_code::NOT_FOUND: 				  return "Not Found";
		case e_code::METHOD_NOT_ALLOWED: 		  return "Method Not Allowed";
		case e_code::LENGTH_REQUIRED: 			  return "Length Required";
		case e_code::CONTENT_TOO_LARGE: 		  return "Content Too Large";
		case e_code::URI_TOO_LONG: 				  return "Uri Too Long";
		case e_code::HTTP_VERSION_NOT_SUPPORTED:  return "Http Version Not Supported";
		case e_code::SERVICE_UNAVAILABLE: 		  return "Service Unavailable";
		default:
			return "";
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
	switch (code)
	{
		case e_code::BAD_REQUEST:
		case e_code::NOT_FOUND:
		case e_code::METHOD_NOT_ALLOWED:
		case e_code::LENGTH_REQUIRED:
		case e_code::CONTENT_TOO_LARGE:
		case e_code::URI_TOO_LONG:
		case e_code::HTTP_VERSION_NOT_SUPPORTED:
		case e_code::SERVICE_UNAVAILABLE:
		  return true;
		default:
			return true;
	}
	return false;
}

bool HttpStatus::is_good(e_code code)
{
	switch (code)
	{
		case e_code::BAD_REQUEST:
		case e_code::NOT_FOUND:
		case e_code::METHOD_NOT_ALLOWED:
		case e_code::LENGTH_REQUIRED:
		case e_code::CONTENT_TOO_LARGE:
		case e_code::URI_TOO_LONG:
		case e_code::HTTP_VERSION_NOT_SUPPORTED:
		case e_code::SERVICE_UNAVAILABLE:
		  return false;
		default:
			return false;
	}
	return true;
}