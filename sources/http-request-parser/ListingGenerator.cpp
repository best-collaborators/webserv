#include "ListingGenerator.hpp"

std::string ListingGenerator::getListingPage( File const & file )
{
	std::string target = file.getRelativePath();
	std::string full_path = file.getFullFilename();

	if (!_isPathValid(full_path))
		return "";

	std::ostringstream html;
	html << "<!DOCTYPE html><html><head><meta charset=UTF-8>"
	     << "<title>Index of " << _escapeHtml(target) << "</title></head><body>"
	     << "<h1>Index of " << _escapeHtml(target) << "</h1><hr>";

	if (target != "/")
		html << "<a href=\"../\">../</a><br>";

	html << _getPathEntries(full_path);
	html << "<hr></body></html>";
	return html.str();
}

bool ListingGenerator::_isPathValid( std::string const & path )
{
	if (std::filesystem::is_directory(path))
		return true;

	Log::warning("Path not valid", "listing");
	return false;
}

std::string ListingGenerator::_getPathEntries( std::string const & path )
{
	std::ostringstream list;
	const std::filesystem::path _path{path};

	try
	{
		for (auto const & entry : std::filesystem::directory_iterator{_path})
		{
			std::string rawName = entry.path().filename().generic_string();
			bool isDir = entry.is_directory();
			std::string display = rawName + (isDir ? "/" : "");
			std::string href = rawName + (isDir ? "/" : "");
			list << _getLink(href, display);
		}
	}
	catch (std::filesystem::filesystem_error const & e)
	{
		Log::warning(e.what(), "listing");
		return "";
	}

	return list.str();
}

std::string ListingGenerator::_getLink( std::string const & href, std::string const & display )
{
	return "<div class=\"entry\"><a href=\"" + href + "\">" + _escapeHtml(display) + "</a></div>";
}

std::string ListingGenerator::_escapeHtml( std::string const & s )
{
	std::string result;
	result.reserve(s.size());
	for (char c : s)
	{
		switch (c)
		{
			case '&':  result += "&amp;";  break;
			case '<':  result += "&lt;";   break;
			case '>':  result += "&gt;";   break;
			case '"':  result += "&quot;"; break;
			case '\'': result += "&#39;";  break;
			default:   result += c;        break;
		}
	}
	return result;
}

