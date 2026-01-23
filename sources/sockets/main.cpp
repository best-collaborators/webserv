#include "../includes/sockets/main.hpp"
#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/Server.hpp"

void	sig_handler(int signum)
{
	if (signum == SIGINT)
		g_running = false;
}

int	main( void )
{
	signal(SIGPIPE, SIG_IGN); //! Set to ignore SIGPIPE signal
	signal(SIGINT, &sig_handler);

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
