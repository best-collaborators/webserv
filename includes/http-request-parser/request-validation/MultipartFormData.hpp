#ifndef MULTIPART_FORM_DATA_PARSER_HPP
#define MULTIPART_FORM_DATA_PARSER_HPP

#include <string>
#include <iostream>

class MultipartFormData
{
private:
	std::string _content_type;
	std::string _content;
	std::string _name;
	std::string _filename = "";

public:

	MultipartFormData(std::string content_type, std::string name, std::string filename = "text");

	MultipartFormData() = delete;
	MultipartFormData(const MultipartFormData &other) = default;
	MultipartFormData(MultipartFormData &&other) = default;
	MultipartFormData & operator=( MultipartFormData && ) noexcept = default;
	~MultipartFormData() = default;

	const std::string &		get_content_type() const;
	void 					set_content_type(const std::string &content_type);

	const std::string &		get_name() const;
	void 					set_name(const std::string &name);

	const std::string &		get_filename() const;
	void 					set_filename(const std::string &filename);

	const std::string &		get_content() const;
	void 					set_content(const std::string &content);
	void					append_content(const std::string &content, std::streamsize size);

	void					clear();

	void print_all_data();
};

#endif /* MULTIPART_FORM_DATA_PARSER_HPP */
