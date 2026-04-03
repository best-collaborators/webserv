#ifndef HTTP_LIMITS_HPP
#define HTTP_LIMITS_HPP

namespace http::limits {
	constexpr int max_header_value_length = 8192;
	constexpr int max_header_count        = 256;
	constexpr int max_uri_length          = 4096;
	constexpr int min_body_length         = 3;
	constexpr int max_body_length         = 100;
	constexpr int upload_file_modulo      = 3;

}

#endif
