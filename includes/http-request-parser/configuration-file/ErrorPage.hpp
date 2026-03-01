#ifndef ERROR_PAGE_HPP
#define ERROR_PAGE_HPP

#include <string>
#include "HttpStatus.hpp"

struct HttpPage
{
	std::string			path;
	HttpStatus::e_code	status_code;
};

#endif /* ERROR_PAGE_HPP */
