#include "CGIRequestConfig.hpp"

namespace
{
	void addMethodEnv( std::string value, std::unordered_map<std::string, std::string> & envp )
	{
		envp.insert({ "REQUEST_METHOD", value });
	}

	void addProtocolEnv( std::string value, std::unordered_map<std::string, std::string> & envp )
	{
		envp.insert({ "SERVER_PROTOCOL", value });
	}

	void addHostEnv( std::string value, std::unordered_map<std::string, std::string> & envp )
	{
		size_t	pos = value.find(':');

		if (pos != std::string::npos)
		{
			envp.insert({ "SERVER_NAME", value.substr(0, pos) });
			envp.insert({ "SERVER_PORT", value.substr(pos + 1) });
		}
	}

	void addTargetEnv( std::string value, std::unordered_map<std::string, std::string> & envp )
	{
		size_t	pos = value.find('?');

		if (pos != std::string::npos)
		{
			envp.insert({ "SCRIPT_NAME", value.substr(0, pos) });
			envp.insert({ "QUERY_STRING", value.substr(pos + 1) });
		}
		else
		{
			envp.insert({ "SCRIPT_NAME", value });
			envp.insert({ "QUERY_STRING", "" });
		}
	}

	void addContentEnv( std::string key, std::string value, std::unordered_map<std::string, std::string> & envp )
	{
		if (key == http::headers::CONTENT_TYPE)
			envp.insert({ "CONTENT_TYPE", value });
		else if (key == http::headers::CONTENT_LENGTH)
			envp.insert({ "CONTENT_LENGTH", value });
	}

	std::string	scriptPathResolver( std::unordered_map<std::string, std::string> const & headers )
	{
		std::regex	reg_ex("(\\w+\\.(?:js|py|php|cgi))");
		std::string	target = headers.at("request-target-decoded");
		std::cout << "filename: " << target << std::endl;

		return target;
	}

	std::vector<std::string> buildEnvp( std::unordered_map<std::string, std::string> const & headers )
	{
		std::vector <std::string> env_vars;
		std::unordered_map<std::string, std::string> envp;

		std::string method = headers.at("method");

		for (auto && header : headers)
		{
			std::string key = header.first;
			std::string value = header.second;

			if (method == "POST")
				addContentEnv(key, value, envp);

			if (key == http::headers::METHOD)
				addMethodEnv(value, envp);
			else if (key == http::headers::REQUEST_TARGET)
				addTargetEnv(value, envp);
			else if (key == http::headers::VERSION)
				addProtocolEnv(value, envp);
			else if (key == http::headers::HOST)
				addHostEnv(value, envp);

			std::cout << key << ": " << value << std::endl;
		}

		std::cout << "\n\nENV VARIABLES: " << std::endl;
		for (auto && [key, value] : envp)
		{
			env_vars.push_back(key + "=" + value);
			std::cout << key << ": " << value << std::endl;
		}
		std::cout << "\n\n" << std::endl;

		return env_vars;
	}
}

bool	cgi::isCGITarget( std::string const & target )
{
	const std::regex regex("(^/cgi-bin/\\w+.(?:js|py|php|cgi))");
	std::smatch match;

	if (std::regex_search(target, match, regex) && match.ready())
		return true;

	return false;
}

CGIConfig cgi::buildConfig( std::unordered_map<std::string, std::string> const & headers )
{
	std::string	executable;

	std::regex	reg_ex("(\\.(?:js|py|php|cgi))");
	std::string	target = headers.at("request-target");
	std::cout << "CGI target: " << target << std::endl;
	std::string extension = RegexMatcher::get_regex_value(target, reg_ex);


	if (extension.empty())
		std::cout << "(cgi::buildConfig) extension.empty()" << std::endl;

	std::cout << "CGI extension: " << extension << std::endl;

	if (extension == ".js")
		executable = "/usr/local/bin/node";
	else if (extension == ".py")
		executable = "/usr/local/bin/python";
	else if (extension == ".php")
		executable = "/usr/bin/php";

	std::string	scriptPath = scriptPathResolver(headers);
	std::vector<std::string> envVariables = buildEnvp(headers);

	CGIConfig	config = {
		executable,
		scriptPath,
		envVariables
	};

	return config;
}
