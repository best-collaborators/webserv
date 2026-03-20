#ifndef ERROR_PAGE_HPP
#define ERROR_PAGE_HPP

#include <string>
#include <filesystem>

#include "HttpStatus.hpp"

struct HttpPage
{
	std::filesystem::path	path;
	HttpStatus::e_code		status_code;
};

#endif /* ERROR_PAGE_HPP */
