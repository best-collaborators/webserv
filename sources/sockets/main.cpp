#include "main.hpp"

static void	sig_handler(int signum)
{
	if (signum == SIGINT)
		g_running = false;
}

int	main(int argc, char **argv)
{
	if (argc < 2) return 1;

	std::unordered_map<ListenData, ServerBlock, ListenDataHash> server_blocks;
	ConfigurationFileParser parser(argv[1], server_blocks);
	if (parser.parse() == ConfigurationFileParser::e_parse_result::ERROR) {
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, &sig_handler);

	try
	{
		Server	server(server_blocks);

		server.run();
	}
	catch( std::exception const & e )
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}
