#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <optional>
#include "HttpPage.hpp"
#include "HttpMethodRegistry.hpp"

class File
{
private:
	std::string					_relative_path;
	std::filesystem::path		_full_filename;
	std::filesystem::path		_directory;
	bool 						_is_dir = false;
	bool 						_autoindex = false;
	std::string					_extension = "";
	std::optional<std::string>	_pass_to;
	HttpPage					_return_page;
	HttpMethodRegistry			_method_registry;
	size_t						_max_body_size = 0;
	bool						_is_index;
	std::string					_path_info = "";


public:
	File(/* args */);
	~File();

	const std::filesystem::path			&getFullFilename() const;
	void								setFullFilename(const std::string& filename);
	const std::filesystem::path			&getDirectory() const;
	void								setDirectory(const std::string& filename);
	const std::string					&getRelativePath() const;
	void								setRelativePath(const std::string& path);
	bool								isDir() const;
	void								setIsDir(bool dir);
	bool								isIndex() const;
	void								setIsIndex(bool dir);
	bool 								getAutoindex() const;
	void								setAutoindex(bool index);
	const std::string					&getExtension() const;
	void 								setExtension(const std::string& ext);
	const std::optional<std::string> 	&getPassTo() const;
	void								setPassTo(const std::optional<std::string>& pass);
	const HttpPage						&getReturnPage() const;
	void								setReturnPage(HttpPage _return_page);
	const HttpMethodRegistry			&getMethodRegistry() const;
	void								setMethodRegistry(HttpMethodRegistry _method_registry);
	size_t								getMaxBodySize() const;
	void								setMaxBodySize(size_t max_body_size);
	const std::string					&getPathInfo() const;
	void								setPathInfo(const std::string &path_info);
};

std::ostream& operator<<(std::ostream& os, const File& file);

#endif /* FILE_HPP */