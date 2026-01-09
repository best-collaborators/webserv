#include "../includes/sockets/main.hpp"
#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/EventLoop.hpp"

int	main( void )
{
	signal(SIGPIPE, SIG_IGN); //! Set to ignore SIGPIPE signal

	Listener	listener(PORT);

	try
	{
		listener.init();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	EventLoop	eventLoop(listener.getListenFd());

	try
	{
		eventLoop.init();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	eventLoop.monitor();

	return 0;
}
