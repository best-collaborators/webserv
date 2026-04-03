#ifndef HTTP_CONTENT_TYPE_HPP
#define HTTP_CONTENT_TYPE_HPP

#include <string>
#include <filesystem>

class HttpContentType {
public:
	enum class e_code : short {
		TEXT_XML,
		TEXT_CSS,
		TEXT_CSV,
		TEXT_JAVASCRIPT,
		TEXT_HTML,
		TEXT_PLAIN,
		IMAGE_JPEG,
		IMAGE_PNG,
		IMAGE_JPG,
		IMAGE_GIF,
		IMAGE_WEBP,
		IMAGE_ICON,
		APPLICATION_JSON,
		APPLICATION_OCTET_STREAM
	} t_code;

	static std::string_view to_string(e_code s);
	static HttpContentType::e_code to_code(const std::string &filename);
	static std::string get_extension_by_content_type(std::string_view content_type);
};

#endif /* HTTP_CONTENT_TYPE_HPP */
