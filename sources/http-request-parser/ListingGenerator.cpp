#include "ListingGenerator.hpp"

std::string ListingGenerator::getListingPage( std::string const & path )
{
	std::string _path = path;

	if (path != "/") //! Check what to do on root path
		_path = path.substr(1);

	std::string html =
	"<!DOCTYPE html><html><head><meta charset=UTF-8>"
	"<title>Index of " + _path + "</title></head><body>"
	"<h1>Index of " + _path + "</h1><hr>";

	if (_path != "/")
		html += "<a href=\"../\">../</a><br>";

	if (!_isPathValid(_path))
		return "";

	std::string list = _getPathEntries(_path);

	html += list;

	html += "<hr></body></html>";
	return html;
}

bool ListingGenerator::_isPathValid( std::string const & path )
{
	if (std::filesystem::is_directory(path))
		return true;
	return false;
}

std::string ListingGenerator::_getPathEntries(std::string const &path)
{
	std::string list;
	const std::filesystem::path _path{path};

	for (auto const & entry : std::filesystem::directory_iterator{_path})
	{
		std::string filename = entry.path().filename().generic_string();

		if (entry.is_directory())
			filename += '/';

		list += _getLink(filename);
	}

	return list;
}

std::string ListingGenerator::_getLink( std::string const & filename )
{
	std::string link = "<div class=\"entry\"><a href=\"" + filename + "\">" + filename + "</a></div>";

	return link;
}