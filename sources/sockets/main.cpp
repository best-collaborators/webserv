#include "../includes/sockets/main.hpp"
#include "../includes/sockets/Listener.hpp"
#include "../includes/sockets/Server.hpp"

#include "ConfigurationFileParser.hpp"

void	sig_handler(int signum)
{
	if (signum == SIGINT)
		g_running = false;
}

int	main( int argc, char *argv[] )
{
	if (argc < 2) return 1;
	ConfigurationFileParser parser(argv[1]);
	if (parser.parse() == ConfigurationFileParser::e_parse_result::ERROR) {
		return 1;
	}

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
