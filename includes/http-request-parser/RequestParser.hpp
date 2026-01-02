#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <regex>

class RequestParser {

	public:
		uint get_status_code();
		class MultipartFormData
		{
			private:
				std::string _content_type;
				std::string _content;
				std::string _name;
				std::string _filename = "";

			public:
				MultipartFormData(std::string content_type, std::string name, std::string filename = "fsadfas");
				MultipartFormData();

				const std::string &get_content_type() const;
				void set_content_type(const std::string &content_type);

				const std::string &get_name() const;
				void set_name(const std::string &name);

				const std::string &get_filename() const;
				void set_filename(const std::string &filename);

				const std::string &get_content() const;
				void set_content(const std::string &content);
				void append_content(const std::string &content);

				void print_all_data();
		};
		RequestParser(std::unordered_map<std::string, std::string> &http_request_values);

	private:
		// MultipartFormData multipartFormData;
		std::string buffer;
		std::unordered_map<std::string, std::string> &_http_request_values;

		const char *ERROR_HTTP_METHOD = "LOG: ERROR INVALID REQUEST METHOD";
		const char *ERROR_HTTP_REQUEST_TARGET = "LOG: ERROR INVALID REQUEST TARGET";
		const char *ERROR_HTTP_VESRION = "LOG: ERROR INVALID REQUEST VERSION";

		const char *REGEX_HTTP_METHOD = "(^[A-Z]{1,32}[ ]+)";
		const char *REGEX_HTTP_REQUEST_TARGET = "(^\\/\\S+[ ]+)";
		const char *REGEX_HTTP_VESRION = "(HTTP\\/(\\d)+.(\\d)+\\s*$)";

		inline std::string &ltrim(std::string &s);
		inline std::string &rtrim(std::string &s);
		inline std::string &trim(std::string &s);

		inline std::string &ltrim(std::string &s, char delim);
		inline std::string &rtrim(std::string &s, char delim);
		inline std::string &trim(std::string &s, char delim);

		std::string get_regex_value(std::string &line, std::regex regex_method);

		bool is_valid_request_line();
		bool add_value_to_map(const char *regex_str, std::string errmsg, std::string key);

		bool is_valid_header();
		int content_length_validation();

		int parse_multipart_form();
		std::string get_multipart_form_boundary();

		bool check_multipart_content_type(RequestParser::MultipartFormData &multipart_form_data);
		bool check_multipart_header(RequestParser::MultipartFormData &multipart_form_data);
};
