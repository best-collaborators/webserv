#pragma once

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signalfd.h>

#include <iostream>

class ChildSignalHandler
{
private:
	int		_sig_fd;

	void	_readSignalFD() noexcept;

public:
	ChildSignalHandler();
	~ChildSignalHandler();

	int		getFD() const noexcept;
	void	handleFinishedChildren() noexcept;
};

