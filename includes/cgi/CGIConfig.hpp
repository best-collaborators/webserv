#pragma once

#include <iostream>
#include <vector>

struct CGIConfig
{
	std::string	executable;
	std::string	scriptPath;
	std::vector<std::string> envVariables;
};