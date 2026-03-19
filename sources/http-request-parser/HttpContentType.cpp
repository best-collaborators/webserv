#include "HttpContentType.hpp"

std::string_view HttpContentType::to_string(HttpContentType::e_code code)
{
	switch (code)
	{
		case HttpContentType::e_code::TEXT_XML:
			return "text/xml";
		case HttpContentType::e_code::TEXT_CSS:
			return "text/css";
		case HttpContentType::e_code::TEXT_CSV:
			return "text/csv";
		case HttpContentType::e_code::TEXT_JAVASCRIPT:
			return "text/javascript";
		case HttpContentType::e_code::TEXT_HTML:
			return "text/html";
		case HttpContentType::e_code::TEXT_PLAIN:
			return "text/plain";
		case HttpContentType::e_code::IMAGE_JPEG:
			return "image/jpeg";
		case HttpContentType::e_code::IMAGE_PNG:
			return "image/png";
		case HttpContentType::e_code::IMAGE_JPG:
			return "image/jpg";
		case HttpContentType::e_code::IMAGE_GIF:
			return "image/gif";
		case HttpContentType::e_code::IMAGE_WEBP:
			return "image/webp";
		case HttpContentType::e_code::IMAGE_ICON:
			return "image/icon";
		case HttpContentType::e_code::APPLICATION_JSON:
			return "application/json";
		default:
			return "application/octet-stream";
	}
}

HttpContentType::e_code HttpContentType::to_code(const std::string &filename)
{
	std::filesystem::path path = filename;
	auto extension = path.extension();

	if (extension == ".xml")
		return HttpContentType::e_code::TEXT_XML;
	if (extension == ".css")
		return HttpContentType::e_code::TEXT_CSS;
	if (extension == ".csv")
		return HttpContentType::e_code::TEXT_CSV;
	if (extension == ".js")
		return HttpContentType::e_code::TEXT_JAVASCRIPT;
	if (extension == ".html" || extension == ".htm")
		return HttpContentType::e_code::TEXT_HTML;
	if (extension == ".txt")
		return HttpContentType::e_code::TEXT_PLAIN;
	if (extension == ".jpeg")
		return HttpContentType::e_code::IMAGE_JPEG;
	if (extension == ".png")
		return HttpContentType::e_code::IMAGE_PNG;
	if (extension == ".jpg")
		return HttpContentType::e_code::IMAGE_JPG;
	if (extension == ".gif")
		return HttpContentType::e_code::IMAGE_GIF;
	if (extension == ".webp")
		return HttpContentType::e_code::IMAGE_WEBP;
	if (extension == ".ico")
		return HttpContentType::e_code::IMAGE_ICON;
	if (extension == ".json")
		return HttpContentType::e_code::APPLICATION_JSON;
	return HttpContentType::e_code::APPLICATION_OCTET_STREAM;
}

std::string HttpContentType::get_extension_by_content_type(std::string_view content_type)
{
	if (content_type == "text/xml")
		return ".xml";
	if (content_type == "text/css")
		return ".css";
	if (content_type == "text/csv")
		return ".csv";
	if (content_type == "text/javascript")
		return ".js";
	if (content_type == "text/html")
		return ".html";
	if (content_type == "text/plain")
		return ".txt";
	if (content_type == "image/jpeg")
		return ".jpeg";
	if (content_type == "image/png")
		return ".png";
	if (content_type == "image/jpg")
		return ".jpg";
	if (content_type == "image/gif")
		return ".gif";
	if (content_type == "image/webp")
		return ".webp";
	if (content_type == "image/icon")
		return ".ico";
	if (content_type == "application/json")
		return ".json";
	return ".bin";
}