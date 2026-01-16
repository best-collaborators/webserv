#include "MultipartFormData.hpp"

MultipartFormData::MultipartFormData(std::string content_type, std::string name, std::string filename)
: _content_type(content_type), _name(name), _filename(filename) {

	std::cout << _content_type <<" "<<_name << " " << _filename << std::endl;
}

MultipartFormData::MultipartFormData() {}

MultipartFormData::~MultipartFormData() {}

void MultipartFormData::print_all_data()
{
	std::cout << "Content-Type: " << _content_type << std::endl;
	std::cout << "Name: " << _name << std::endl;
	std::cout << "Filename: " << _filename << std::endl;
	std::cout << "Content: " << _content << std::endl;
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
