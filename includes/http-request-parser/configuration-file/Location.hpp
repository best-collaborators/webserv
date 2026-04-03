#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include "HttpPage.hpp"
#include "HttpMethodRegistry.hpp"
#include <optional>
#include <iostream>
#include <filesystem>

class Location
{
	std::filesystem::path				path;
	std::filesystem::path				root;
	bool								autoindex = false;
	std::string							default_file;
	std::optional<HttpMethodRegistry>	methods_registry;
	HttpPage							return_page;
	size_t								_max_body_size = 0;

	public:
		// Getters
		const std::filesystem::path& getPath() const;
		const std::filesystem::path& getRoot() const;
		bool getAutoindex() const;
		const std::string& getDefaultFile() const;
		const std::optional<HttpMethodRegistry>& getMethodsRegistry() const;
		const HttpPage& getReturnPage() const;
		size_t getMaxBodySize() const;

		// Setters
		void setPath(const std::filesystem::path& p);
		void setRoot(const std::filesystem::path& r);
		void setAutoindex(bool a);
		void setDefaultFile(const std::string& f);
		void setMethodsRegistry(const HttpMethodRegistry& m);
		void setMaxBodySize(const size_t max_body_size);
		std::string to_string() const noexcept;
		void setReturnPage(const HttpPage& p);
};

std::ostream& operator<<(std::ostream& os, const Location& location);

#endif /* LOCATION_HPP */

