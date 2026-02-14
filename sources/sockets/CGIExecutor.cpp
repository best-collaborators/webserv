#include "CGIExecutor.hpp"

CGIExecutor::CGIExecutor( CGIConfig & config ): _pid(-1)
{
	_initPipes();
	_initFork();
	_initChild(config);

	_write_pipe[STDIN_FILENO].reset();
	_read_pipe[STDOUT_FILENO].reset();
}

int CGIExecutor::getPID() const noexcept
{
	return _pid;
}

int CGIExecutor::releaseWriteFD() noexcept
{
	return _write_pipe[STDOUT_FILENO].release();
}

int CGIExecutor::releaseReadFD() noexcept
{
	return _read_pipe[STDIN_FILENO].release();
}

void CGIExecutor::_initPipes()
{
	int	wp[2];

	if (pipe(wp) == -1)
		throw std::system_error(errno, std::generic_category(), "[CGI] write pipe creation failed");

	_write_pipe[0] = PipeFD(wp[0]);
	_write_pipe[1] = PipeFD(wp[1]);

	int	rp[2];

	if (pipe(rp) == -1)
	{
		throw std::system_error(errno, std::generic_category(), "[CGI] read pipe creation failed");
	}

	_read_pipe[0] = PipeFD(rp[0]);
	_read_pipe[1] = PipeFD(rp[1]);
}

void CGIExecutor::_initFork()
{
	_pid = fork();

	if (_pid == -1)
		throw std::system_error(errno, std::generic_category(), "[CGI] fork creation failed");
}

void CGIExecutor::_initChild( CGIConfig & config )
{
	if (_pid == 0)
	{
		try
		{
			
			_restoreDefaultSignalMask();
	
			_initChildPipes();
	
			std::vector<char *>	envp = _generateEnvp(config);
			std::vector<char *>	argv = _generateArgv(config);
	
			if (execve(config.executable.data(), argv.data(), envp.data()) == -1)
			{
				std::cerr << "[child-CGI] execve failed" << std::endl;
				_exit(1);
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			_exit(1);
		}
	}
}

void CGIExecutor::_restoreDefaultSignalMask()
{
	sigset_t	empty_mask;
	sigemptyset(&empty_mask);
	sigprocmask(SIG_SETMASK, &empty_mask, NULL);
}

void CGIExecutor::_initChildPipes()
{
	// Write pipe
	_write_pipe[STDOUT_FILENO].reset();
	_dup2FD(_write_pipe[STDIN_FILENO].get(), STDIN_FILENO);
	_write_pipe[STDIN_FILENO].reset();

	// Read pipe
	_read_pipe[STDIN_FILENO].reset();
	_dup2FD(_read_pipe[STDOUT_FILENO].get(), STDOUT_FILENO);
	_read_pipe[STDOUT_FILENO].reset();
}

void CGIExecutor::_dup2FD( int fd1, int fd2 )
{
	if (dup2(fd1, fd2) == -1)
		throw std::system_error(errno, std::generic_category(), "[CGI] dup2 failed");
}

std::vector<char *>	CGIExecutor::_generateEnvp( CGIConfig & config )
{
	std::vector<char *>	envp;

	for (size_t i = 0; i < config.envVariables.size(); i++)
		envp.push_back(config.envVariables[i].data());
	envp.push_back(nullptr);

	return envp;
}

std::vector<char *> CGIExecutor::_generateArgv( CGIConfig & config )
{
	std::vector<char *>	argv = {
		config.executable.data(),
		config.scriptPath.data(),
		nullptr
	};

	return argv;
}
