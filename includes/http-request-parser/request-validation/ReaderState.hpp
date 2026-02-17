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

#endif /* READER_STATE */
