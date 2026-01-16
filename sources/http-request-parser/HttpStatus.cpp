#include "HttpStatus.hpp"

std::string_view HttpStatus::get_status_code_name(e_code s)
{
	switch (s)
	{
		case e_code::OK:
			return "OK";
			break;
		case e_code::BAD_REQUEST:
			return "Bad Request";
			break;
		case e_code::CREATED:
			return "Created";
			break;
		case e_code::NOT_FOUND:
			return "Not found";
			break;
		case e_code::METHOD_NOT_ALLOWED:
			return "Method Not Allowed";
			break;
		case e_code::LENGTH_REQUIRED:
			return "Length Required";
			break;
		case e_code::CONTENT_TOO_LARGE:
			return "Content Too Large";
			break;
		case e_code::URI_TOO_LONG:
			return "Uri Too Long";
			break;
		case e_code::HTTP_VERSION_NOT_SUPPORTED:
			return "Http Version Not Supported";
			break;
		case e_code::SERVICE_UNAVAILABLE:
			return "Service Unavailable";
			break;
		default:
			return "";
			break;
	}
}
