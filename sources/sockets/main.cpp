#include "../includes/sockets/main.hpp"
#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/Server.hpp"

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

	try
	{
		Server	Server(listener.getListenFd());

		Server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}


	return 0;
}
