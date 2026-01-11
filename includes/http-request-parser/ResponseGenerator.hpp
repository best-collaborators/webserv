#ifndef RESPONSE_GENERATOR_HPP
#define RESPONSE_GENERATOR_HPP

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
		std::string_view  _method;
		bool			  _is_a_file;

		std::string get_file_last_modified_date(const char *filename);
		std::_Put_time<char> get_date_GMT();
		std::string serve_html_error_page(std::string errmsg);
		std::string serve_html_success_page(std::string message);
		std::string create_body();

	public:
		ResponseGenerator() = delete;
		ResponseGenerator(uint status_code, std::string content, std::string_view content_type, std::string_view _method, bool is_a_file);
		void form_reponse(bool is_a_file);
		void form_reponse();
};

#endif /* RESPONSE_GENERATOR_HPP */
