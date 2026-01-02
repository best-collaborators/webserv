#include "../includes/sockets/main.hpp"
#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/EventLoop.hpp"

int	main( void )
{
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
