#include "File.hpp"

File::File(/* args */)
{
}

File::~File()
{
}

const std::filesystem::path& File::getFullFilename() const {
	return _full_filename;
}

void File::setFullFilename(const std::string& filename) {
	_full_filename = filename;
}

const std::filesystem::path			&File::getDirectory() const
{
	return _directory;
}

void								File::setDirectory(const std::string& filename)
{
	_directory = filename;
}


const std::string &File::getRelativePath() const {
	return _relative_path;
}

void File::setRelativePath(const std::string& path) {
	_relative_path = path;
}

bool File::isDir() const {
	return _is_dir;
}

void File::setIsDir(bool dir) {
	_is_dir = dir;
}

bool File::getAutoindex() const {
	return _autoindex;
}

void File::setAutoindex(bool index) {
	_autoindex = index;
}

const std::string& File::getExtension() const {
	return _extension;
}

void File::setExtension(const std::string& ext) {
	_extension = ext;
}

const std::optional<std::string>& File::getPassTo() const {
	return _pass_to;
}

void File::setPassTo(const std::optional<std::string>& pass) {
	_pass_to = pass;
}

const HttpPage &File::getReturnPage() const
{
	return _return_page;
}

void File::setReturnPage(HttpPage return_page)
{
	_return_page = return_page;
}

const HttpMethodRegistry &File::getMethodRegistry() const
{
	return _method_registry;
}

void File::setMethodRegistry(HttpMethodRegistry method_registry)
{
	_method_registry = method_registry;
}

size_t File::getMaxBodySize() const
{
	return _max_body_size;
}

void File::setMaxBodySize(size_t max_body_size)
{
	_max_body_size = max_body_size;
}

bool File::isIndex() const
{
	return _is_index;
}

void File::setIsIndex(bool is_index)
{
	_is_index = is_index;
}


const std::string &File::getPathInfo() const
{
	return _path_info;
}

void File::setPathInfo(const std::string &path_info)
{
	_path_info = path_info;
}

std::ostream& operator<<(std::ostream& os, const File& file)
{
	os << "File: " << file.getFullFilename() << "\n"
	   << "  Relative Path: " << file.getRelativePath() << "\n"
	   << "  Directory: " << file.getDirectory() << "\n"
	   << "  Is Directory: " << (file.isDir() ? "true" : "false") << "\n"
	   << "  Autoindex: " << (file.getAutoindex() ? "true" : "false") << "\n"
	   << "  Extension: " << file.getExtension() << "\n"
	   << "  Is Index: " << (file.isIndex() ? "true" : "false") << "\n"
	   << "  Path Info: " << file.getPathInfo() << "\n"
	   << "  Max Body Size: " << file.getMaxBodySize() << "\n";
	   if (file.getReturnPage().path.empty()) {
		std::cout << "  Relocation: " << file.getReturnPage().status_code
		<< " " << file.getReturnPage().path << "\n";
	   }
	
	if (file.getPassTo().has_value()) {
		os << "  Pass To: " << file.getPassTo().value() << "\n";
	}
	
	return os;
}
