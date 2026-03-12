#include "File.hpp"

File::File(/* args */)
{
}

File::~File()
{
}

const std::string& File::getFullFilename() const {
	return _full_filename;
}

void File::setFullFilename(const std::string& filename) {
	_full_filename = filename;
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
