#include "MultipartFormData.hpp"

MultipartFormData::MultipartFormData(std::string content_type, std::string name, std::string filename)
: _content_type(content_type), _name(name), _filename(filename) {

	Log::debug(_content_type + " " + _name + " " + _filename, "http-parser");
}

void MultipartFormData::print_all_data()
{
	Log::debug("Content-Type: " + _content_type, "http-parser");
	Log::debug("Name: " + _name, "http-parser");
	Log::debug("Filename: " + _filename, "http-parser");
	Log::debug("Content: " + _content, "http-parser");
}

const std::string &MultipartFormData::get_content_type() const {
	return _content_type;
}

void MultipartFormData::set_content_type(const std::string &content_type) {
	_content_type = content_type;
}

const std::string &MultipartFormData::get_name() const {
	return _name;
}

void MultipartFormData::set_name(const std::string &name) {
	_name = name;
}

const std::string &MultipartFormData::get_filename() const {
	return _filename;
}

const std::string &MultipartFormData::get_content() const {
	return _content;
}

void MultipartFormData::set_filename(const std::string &filename) {
	_filename = filename;
}

void MultipartFormData::set_content(const std::string &content) {
	_content = content;
}

void MultipartFormData::append_content(const std::string &content, std::streamsize size) {
	_content.append(content + "\n", _content.size(), size);
}

void MultipartFormData::clear()
{
	_filename.clear();
	_name.clear();
	_content.clear();
}
