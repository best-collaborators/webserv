#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <optional>
#include "HttpPage.hpp"

class File
{
private:
	std::string					_full_filename;
	bool 						_is_dir = false;
	bool 						_autoindex = false;
	std::string					_extension;
	std::optional<std::string>	_pass_to;
	HttpPage					_return_page;

public:
	File(/* args */);
	~File();

	const std::string					&getFullFilename() const;
	void								setFullFilename(const std::string& filename);
	bool								isDir() const;
	void								setIsDir(bool dir);
	bool 								getAutoindex() const;
	void								setAutoindex(bool index);
	const std::string					&getExtension() const;
	void 								setExtension(const std::string& ext);
	const std::optional<std::string> 	&getPassTo() const;
	void								setPassTo(const std::optional<std::string>& pass);
	const HttpPage						&getReturnPage() const;
	void								setReturnPage(HttpPage _return_page);
};

#endif /* FILE_HPP */