#include "main.hpp"

static void	sig_handler(int signum)
{
	if (signum == SIGINT)
		g_running = false;
}

int	main(int argc, char **argv)
{
	if (argc > 2) {
		std::cerr << "Invalid webserv execution. Usage: ./webserv [custom.conf]" << std::endl;
		return 1;
	}

	std::string config_filename = argv[1] ? argv[1] : "";
	std::unordered_map<ListenData, ServerBlock, ListenDataHash> server_blocks;
	ConfigurationFileParser parser(config_filename, server_blocks);
	if (parser.parse() == ConfigurationFileParser::e_parse_result::ERROR) {
		return 1;
	}
	for (auto server_block : server_blocks) {
		Log::info(to_string(server_block.second), "config-file");
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
		Log::critical(e.what(), "server");
		return 1;
	}

	return 0;
}
