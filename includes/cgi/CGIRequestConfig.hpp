#pragma once

#include <regex>
#include <string_view>

#include "Request.hpp"
#include "CGIConfig.hpp"
#include "ServerBlock.hpp"
#include "RegexMatcher.hpp"

class CGIConfigBuilder {
public:
	static CGIConfig build(
		const std::unordered_map<std::string, std::string>& headers,
		const File& file);

private:
	static bool isCGIHeader(std::string_view key);
	static std::string toCGIHeaderName(std::string_view headerName);
	static void addHostEnv(
		std::string_view value,
		std::unordered_map<std::string, std::string>& envp);
	static void addTargetEnv(
		std::string_view value,
		std::unordered_map<std::string, std::string>& envp);
	static std::vector<std::string> buildEnvp(
		const std::unordered_map<std::string, std::string>& headers);
};