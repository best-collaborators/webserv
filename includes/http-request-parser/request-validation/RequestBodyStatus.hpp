#ifndef REQUEST_BODY_STATUS_HPP
#define REQUEST_BODY_STATUS_HPP

#include <ostream>

enum class RequestType
{
	NO_BODY,
	RAW_BODY,
	MULTIPART,
	CHUNKED,
	CGI
};

inline std::ostream& operator<<(std::ostream& os, RequestType status)
{
	switch (status)
	{
		case RequestType::NO_BODY:
			os << "NO_BODY";
			break;
		case RequestType::RAW_BODY:
			os << "RAW_BODY";
			break;
		case RequestType::MULTIPART:
			os << "MULTIPART";
			break;
		case RequestType::CHUNKED:
			os << "CHUNKED";
			break;
		case RequestType::CGI:
			os << "CGI";
			break;
	}
	return os;
}

#endif /* REQUEST_BODY_STATUS_HPP */