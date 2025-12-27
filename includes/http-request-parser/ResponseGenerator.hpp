#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <chrono>
#include <iomanip>
#include "HttpStatus.hpp"

class ResponseGenerator
{
	private:
		uint 			  _status_code;
		std::string 	  _content;
		std::string_view  _content_type;
		std::streampos	  _length;
		bool			  _is_a_file;

		std::string get_file_last_modified_date(const char *filename);
		std::_Put_time<char> get_date_GMT();
		std::string serve_html_error_page(std::string errmsg);
		std::string create_body();

	public:
		ResponseGenerator() = delete;
		ResponseGenerator(uint status_code, std::string content, std::string_view content_type, bool is_a_file);
		void form_reponse(bool is_a_file);
		void form_reponse();
};