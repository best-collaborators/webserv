#ifndef REQUEST_BODY_STATUS_HPP
#define REQUEST_BODY_STATUS_HPP

#include <ostream>

enum class RequestBodyStatus
{
	NO_BODY,
	RAW_BODY,
	MULTIPART,
	CHUNKED,
	CGI
};

inline std::ostream& operator<<(std::ostream& os, RequestBodyStatus status)
{
	switch (status)
	{
		case RequestBodyStatus::NO_BODY:
			os << "NO_BODY";
			break;
		case RequestBodyStatus::RAW_BODY:
			os << "RAW_BODY";
			break;
		case RequestBodyStatus::MULTIPART:
			os << "MULTIPART";
			break;
		case RequestBodyStatus::CHUNKED:
			os << "CHUNKED";
			break;
		case RequestBodyStatus::CGI:
			os << "CGI";
			break;
	}
	return os;
}

#endif /* REQUEST_BODY_STATUS_HPP */