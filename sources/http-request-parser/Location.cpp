#include "Location.hpp"

const std::filesystem::path& Location::getPath() const {
	return path;
}

const std::filesystem::path& Location::getRoot() const {
	return root;
}

bool Location::getAutoindex() const {
	return autoindex;
}

const std::string& Location::getDefaultFile() const {
	return default_file;
}

const std::optional<HttpMethodRegistry>& Location::getMethodsRegistry() const {
	return methods_registry;
}

const HttpPage& Location::getReturnPage() const {
	return return_page;
}

void Location::setPath(const std::filesystem::path& p) {
	path = p;
}

void Location::setRoot(const std::filesystem::path& r) {
	root = r;
}

void Location::setAutoindex(bool a) {
	autoindex = a;
}

void Location::setDefaultFile(const std::string& f) {
	default_file = f;
}

void Location::setMethodsRegistry(const HttpMethodRegistry& m) {
	if (!methods_registry.has_value())
		methods_registry.emplace();
	methods_registry.value() = m;
}

void Location::setReturnPage(const HttpPage& p) {
	return_page = p;
}

std::ostream& operator<<(std::ostream& os, const Location& location)
{
	Logger::displayLog(Logger::e_log_level::INFO, "Location path: " + location.getPath().string(), "config");

	std::optional<HttpMethodRegistry> copy = location.getMethodsRegistry();
	if (copy.has_value())
		location.getMethodsRegistry()->printAllowedMethods();
	if (!location.getDefaultFile().empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Index: " + location.getDefaultFile(), "config");
	if (!location.getRoot().empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Root: " + location.getRoot().string(), "config");
	Logger::displayLog(Logger::e_log_level::INFO, "Max body size: " + std::to_string(location.getMaxBodySize()), "config");

	if (!location.getReturnPage().path.empty())
		Logger::displayLog(Logger::e_log_level::INFO, "Return page: "
				+ std::to_string((int)location.getReturnPage().status_code)
				+ " "
				+ location.getReturnPage().path.string(), "config");
	std::string autoindex_enabled = (location.getAutoindex() ? "true" : "false");
	Logger::displayLog(Logger::e_log_level::INFO, "Autoindex: " + autoindex_enabled, "config");

	return os;
}

std::string Location::to_string() const noexcept
{
	std::string result = "Location path: " + path.string() + "\n";
	
	if (methods_registry.has_value())
		result += "    Methods: " + methods_registry.value().to_string() + "\n";
	if (!default_file.empty())
		result += "    Index: " + default_file + "\n";
	result += "    Max body size: " + std::to_string(getMaxBodySize());
	if (!root.empty())
		result += "    Root: " + root.string() + "\n";
	if (!return_page.path.empty())
	result += "Return page: "
			+ std::to_string((int)return_page.status_code)
			+ " "
			+ return_page.path.string() + "\n";
	result += "    Autoindex: " + std::string(autoindex ? "true" : "false") + "\n";
	
	return result;
}

size_t Location::getMaxBodySize() const
{
	return _max_body_size;
}

void Location::setMaxBodySize(size_t max_body_size)
{
	_max_body_size = max_body_size;
}