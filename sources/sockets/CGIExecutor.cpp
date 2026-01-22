#include "CGIExecutor.hpp"

CGIExecutor::CGIExecutor()
{
	pid_t	fork_pid;
	int	send_fds[2];
	int recv_fds[2];

	if (pipe(send_fds) == -1)
	{
		std::cerr << "send pipes creation failed" << std::endl;
	}
	if (pipe(recv_fds) == -1)
	{
		std::cerr << "recv pipes creation failed" << std::endl;
	}

	fork_pid = fork();
	if (fork_pid == -1)
	{
		std::cerr << "fork failed" << std::endl;
	}
	else if (fork_pid == 0)
	{
		std::cout << "child" << std::endl;
		close(send_fds[STDOUT_FILENO]);
		close(recv_fds[STDIN_FILENO]);

		dup2(send_fds[STDIN_FILENO], STDIN_FILENO);
		close(send_fds[STDIN_FILENO]);

		dup2(recv_fds[STDOUT_FILENO], STDOUT_FILENO);
		dup2(recv_fds[STDOUT_FILENO], STDERR_FILENO);
		close(recv_fds[STDOUT_FILENO]);

		char file[] = "tests/test.js";
		char env[] = "TEST=Test!";
		char path[] = "/home/rmzvr/.nvm/versions/node/v24.11.1/bin/node";
		char *envp[] = { env, nullptr };
		char *argv[] = { path, file, nullptr };

		if (execve(argv[0], argv, envp) == -1)
		{
			std::cerr << "execve failed" << std::endl;
		}
	}
	else
	{
		std::cout << "parent" << std::endl;

		char	buffer_read[1001];
		char	buffer_write[] = "hi";

		close(send_fds[STDIN_FILENO]);
		close(recv_fds[STDOUT_FILENO]);

		write(send_fds[STDOUT_FILENO], buffer_write, sizeof(buffer_write) - 1);
		close(send_fds[STDOUT_FILENO]);

		ssize_t i = 1;
		while (i > 0)
		{
			i = read(recv_fds[STDIN_FILENO], buffer_read, sizeof(buffer_read));
			if (i > 0)
			{
				buffer_read[i] = '\0';
				std::cout << "buffer_read: " << buffer_read << std::endl;
			}
		}
		close(recv_fds[STDIN_FILENO]);

		wait(NULL);
	}
}

CGIExecutor::~CGIExecutor()
{}
