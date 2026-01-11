#include "Poller.hpp"

Poller::Poller(): _epoll_fd(-1), _events(MAX_EVENTS)
{
	int status = epoll_create1(EPOLL_CLOEXEC); //! Check for forks

	if (status == -1)
	{
		throw std::system_error(errno, std::generic_category(), "[epoll] epoll_create failed");
	}

	_epoll_fd = status;

	std::cout << "[epoll] Instance created: fd " << _epoll_fd << "." << std::endl;
}

Poller::~Poller()
{
	if (_epoll_fd != -1)
	{
		while (close(_epoll_fd) == -1 && errno == EINTR) {}

		_epoll_fd = -1;
	}
}

int		Poller::wait()
{
	int	count = epoll_wait(_epoll_fd, _events.data(), static_cast<int>(_events.size()), TIMEOUT);

	if (count == -1)
	{
		if (errno == EINTR) //! Handle tab resize or debugger
		{
			return 0;
		}

		std::cerr << "[epoll] epoll_wait failed (" << errno << "): " << strerror(errno) << std::endl;
		return 0;
	}
	else if (count > 0)
	{
		std::cout << "\n[epoll] epoll_wait returned " << count << " event(s)." << std::endl;
	}
	return count;
}

bool	Poller::control( int operation, int fd, uint32_t events )
{
	epoll_event	event {};

	event.events = events;
	event.data.fd = fd;

	epoll_event *	event_p = &event;

	if (operation == EPOLL_CTL_DEL)
	{
		event_p = nullptr;
	}

	if (epoll_ctl(_epoll_fd, operation, fd, event_p) == -1)
	{
		std::cerr << "[epoll] Operation " << operation << " for fd " << fd << " failed (" << errno << "): " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

bool	Poller::add( int fd, uint32_t events )
{
	return control(EPOLL_CTL_ADD, fd, events);
}

bool	Poller::mod( int fd, uint32_t events )
{
	return control(EPOLL_CTL_MOD, fd, events);
}

bool	Poller::del( int fd )
{
	return control(EPOLL_CTL_DEL, fd, 0);
}

epoll_event const &	Poller::getEvent( int index ) const
{
	return _events[index];
}