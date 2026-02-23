#ifndef HTTP_CONTENT_TYPE_HPP
#define HTTP_CONTENT_TYPE_HPP

#include <string>

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

	static std::string_view get_content_type_name(e_code s);
	static HttpContentType::e_code get_content_type_code_by_extension(std::string_view extension);
	static std::string get_content_type_by_extension(std::string_view extension);
	static std::string get_extension_by_content_type(std::string_view content_type);
};

#endif /* HTTP_CONTENT_TYPE_HPP */
