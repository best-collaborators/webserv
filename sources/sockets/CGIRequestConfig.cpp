#include "CGIRequestConfig.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace
{
	const std::unordered_set<std::string> CGI_HEADERS = {
		http::headers::METHOD,
		http::headers::VERSION,
		http::headers::REQUEST_TARGET,
		http::headers::REQUEST_TARGET_DECODED,
		http::headers::HOST,
		http::headers::STATUS,
		http::headers::CONTENT_TYPE,
		http::headers::CONTENT_LENGTH,
		http::headers::TRANSFER_ENCODING
	};

	bool isCGIHeader( std::string const & key )
	{
		return CGI_HEADERS.count(key) > 0;
	}

	std::string toCGIHeaderName( std::string const & headerName )
	{
		std::string result = "HTTP_";
		for (char c : headerName)
		{
			if (c == '-')
				result += '_';
			else
				result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
		}
		return result;
	}

	void addHostEnv( std::string const & value, std::unordered_map<std::string, std::string> & envp )
	{
		size_t pos = value.find(':');
		if (pos != std::string::npos)
		{
			envp["SERVER_NAME"] = value.substr(0, pos);
			envp["SERVER_PORT"] = value.substr(pos + 1);
		}
		else
		{
			envp["SERVER_NAME"] = value;
			envp["SERVER_PORT"] = "80";
		}
	}

	void addTargetEnv( std::string const & value, std::unordered_map<std::string, std::string> & envp )
	{
		std::string path = value;
		std::string queryString;

		size_t qpos = value.find('?');
		if (qpos != std::string::npos)
		{
			path = value.substr(0, qpos);
			queryString = value.substr(qpos + 1);
		}

		envp["SCRIPT_NAME"] = path;
		envp["QUERY_STRING"] = queryString;
	}

	std::vector<std::string> buildEnvp( std::unordered_map<std::string, std::string> const & headers )
	{
		std::unordered_map<std::string, std::string> envp;

		envp["GATEWAY_INTERFACE"] = "CGI/1.1";

		if (headers.count(http::headers::METHOD))
			envp["REQUEST_METHOD"] = headers.at(http::headers::METHOD);

		if (headers.count(http::headers::VERSION))
			envp["SERVER_PROTOCOL"] = headers.at(http::headers::VERSION);

		if (headers.count(http::headers::HOST))
			addHostEnv(headers.at(http::headers::HOST), envp);

		if (headers.count(http::headers::REQUEST_TARGET))
			addTargetEnv(headers.at(http::headers::REQUEST_TARGET), envp);

		if (headers.count(http::headers::CONTENT_TYPE))
			envp["CONTENT_TYPE"] = headers.at(http::headers::CONTENT_TYPE);

		if (headers.count(http::headers::CONTENT_LENGTH))
			envp["CONTENT_LENGTH"] = headers.at(http::headers::CONTENT_LENGTH);

		for (auto const & [key, value] : headers)
		{
			if (isCGIHeader(key))
				continue;
			envp[toCGIHeaderName(key)] = value;
		}

		std::vector<std::string> env_vars;
		env_vars.reserve(envp.size());
		for (auto const & [key, value] : envp)
			env_vars.push_back(key + "=" + value);

		return env_vars;
	}
}

CGIConfig cgi::buildConfig( std::unordered_map<std::string, std::string> const & headers, File const & file )
{
	return CGIConfig {
		file.getPassTo().value(),
		file.getFullFilename(),
		buildEnvp(headers)
	};
}
