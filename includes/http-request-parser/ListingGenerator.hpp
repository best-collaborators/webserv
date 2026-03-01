#pragma once

#include "map"
#include "iostream"
#include <filesystem>

class ListingGenerator
{
private:
	static bool _isPathValid( std::string const & path );
	static std::string _getPathEntries( std::string const & path );
	static std::string _getLink( std::string const & filename );

public:
	ListingGenerator() = delete;
	~ListingGenerator() = delete;

	static std::string getListingPage( std::string const & path );
};
