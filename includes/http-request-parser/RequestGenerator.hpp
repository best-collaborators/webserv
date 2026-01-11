#ifndef REQUEST_GENERATOR_HPP
#define REQUEST_GENERATOR_HPP

#include <iostream>
#include <fstream>

class RequestGenerator
{
	private:
		static void create_body_for_request(std::string &body, std::string &file_to_send, std::string &boundary);

	public:
		static void create_post_request(std::string &request);
};

#endif /* REQUEST_GENERATOR_HPP */
