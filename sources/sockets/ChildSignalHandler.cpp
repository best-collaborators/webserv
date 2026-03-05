#include "ChildSignalHandler.hpp"

ChildSignalHandler::ChildSignalHandler(): _sig_fd(-1)
{
	sigset_t	sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);

	if (sigprocmask(SIG_BLOCK, &sigset, nullptr) == -1)
		throw std::system_error(errno, std::generic_category(), "[ChildSignalHandler] sigprocmask failed.");

	_sig_fd = signalfd(-1, &sigset, SFD_NONBLOCK | SFD_CLOEXEC);

	if (_sig_fd == -1)
		throw std::system_error(errno, std::generic_category(), "[ChildSignalHandler] signalfd failed.");
}

ChildSignalHandler::~ChildSignalHandler()
{
	if (_sig_fd != -1)
		close(_sig_fd);
	_sig_fd = -1;
}

void ChildSignalHandler::_readSignalFD() noexcept
{
	ssize_t				read_bytes;
	signalfd_siginfo	status;

	read_bytes = read(_sig_fd, &status, sizeof(status));
	if (read_bytes != sizeof(status))
		std::cerr << "[server] sigfd read failed." << std::endl;
}

int ChildSignalHandler::getFD() const noexcept
{
	return _sig_fd;
}

std::vector<pid_t> ChildSignalHandler::handleFinishedChildren() noexcept
{
	pid_t	pid;
	int		status;

	std::vector<pid_t>	finished;

	_readSignalFD();

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		finished.push_back(pid);
		std::cout << "pid: " << pid << std::endl;
		std::cout << "status: " << status << std::endl;
	}
	return finished;
}