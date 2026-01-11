#pragma once

enum class IoState
{
	Pending,	// Still reading/writing, do nothing
	Closed,		// Peer closed connection
	Error,		// Socket error
	Sent,
	Received
};
