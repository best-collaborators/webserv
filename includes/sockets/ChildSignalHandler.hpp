#pragma once

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/signalfd.h>

#include <vector>
#include <iostream>

#include <Logger.hpp>
class ChildSignalHandler
{
private:
	int		_sig_fd;

	void	_readSignalFD() noexcept;

public:
	ChildSignalHandler();
	~ChildSignalHandler();

	int		getFD() const noexcept;
	std::vector<pid_t>	handleFinishedChildren() noexcept;
};

