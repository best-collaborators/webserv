#pragma once

enum class IoSource
{
	Connection,
	CGI
};

enum class IoEvent
{
	Pending,	// Still reading/writing, do nothing
	Closed,		// Peer closed connection
	Error,		// Socket error
	Sent,		// Sent data
	Received,	// Received data
	Init,		// CGI init
	Done		// CGI done
};

struct IoResult
{
	IoSource	source;
	IoEvent		event;
};
