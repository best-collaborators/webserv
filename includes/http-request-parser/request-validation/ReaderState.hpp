#ifndef READER_STATE
#define READER_STATE


//? ERROR and COMPLETE are the same. Consider removing
enum ReaderState
{
	AwaitingHeaders,
	AwaitingBody,
	Complete,
	Error,
	CGI
};

#include <iostream>

inline std::ostream& operator<<(std::ostream& os, ReaderState state)
{
	switch (state)
	{
		case AwaitingHeaders:
			os << "AwaitingHeaders";
			break;
		case AwaitingBody:
			os << "AwaitingBody";
			break;
		case Complete:
			os << "Complete";
			break;
		case Error:
			os << "Error";
			break;
		case CGI:
			os << "CGI";
			break;
	}
	return os;
}

#endif /* READER_STATE */
