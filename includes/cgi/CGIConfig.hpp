#pragma once

#include <iostream>
#include <vector>

struct CGIConfig
{
	size_t		max_body_size;
	std::string	executable;
	std::string	scriptPath;
	std::vector<std::string> envVariables;
};