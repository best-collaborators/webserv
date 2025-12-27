#include <string>

class HttpStatus {

	public:

		enum class e_code : short
		{
			OK = 200,
			BAD_REQUEST = 400,
			NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			LENGTH_REQUIRED = 411,
			CONTENT_TOO_LARGE = 413,
			URI_TOO_LONG = 414,
			HTTP_VERSION_NOT_SUPPORTED = 505
		} t_code;

		static std::string_view get_status_code_name(e_code s);
};
