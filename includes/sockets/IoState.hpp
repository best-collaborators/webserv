#pragma once

enum class IoState
{
	Pending,	// Still reading/writing, do nothing
	Ready,		// Request/Response complete, switch state (e.g., READ -> WRITE)
	Closed,		// Peer closed connection
	Error		// Socket error
};
