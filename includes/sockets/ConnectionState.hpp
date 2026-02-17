#ifndef CONNECTION_STATE_HPP
#define CONNECTION_STATE_HPP

enum class ConnectionState {
	AwaitingHeaders,
	AwaitingBody,
	FormingResponse,
	SendingResponse,
	CGIProcessing,
	Complete
};

#endif /* CONNECTION_STATE_HPP */