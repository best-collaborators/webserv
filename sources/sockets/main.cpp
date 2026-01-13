#include "../includes/sockets/main.hpp"
#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/Server.hpp"

int	main( void )
{
	signal(SIGPIPE, SIG_IGN); //! Set to ignore SIGPIPE signal

	try
	{
		Server	server(PORT);

		server.run();
	}
	catch( std::exception const & e )
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}
