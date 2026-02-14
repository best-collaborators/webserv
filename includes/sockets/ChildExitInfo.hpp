#pragma once

#include <sys/wait.h>

struct ChildExitInfo
{
	pid_t	pid;
	int		status;

	bool		success() const
	{
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	};
};