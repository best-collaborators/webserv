#pragma once

#include <filesystem>
#include <sstream>

#include "File.hpp"
#include "Logger.hpp"

class ListingGenerator
{
private:
	static bool _isPathValid( std::string const & path );
	static std::string _getPathEntries( std::string const & path );
	static std::string _getLink( std::string const & href, std::string const & display );
	static std::string _escapeHtml( std::string const & s );

public:
	ListingGenerator() = delete;
	~ListingGenerator() = delete;

	static std::string getListingPage( File const & file );
};
