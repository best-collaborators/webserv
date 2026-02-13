#ifndef HTTP_HEADERS
#define HTTP_HEADERS

namespace http::headers {
	constexpr auto HOST = "host";
	constexpr auto CONTENT_LENGTH = "content-length";
	constexpr auto CONTENT_TYPE = "content-type";
	constexpr auto REQUEST_TARGET = "request-target";
	constexpr auto REQUEST_TARGET_DECODED = "request-target-decoded";
	constexpr auto TRANSFER_ENCODING = "transfer-encoding";
	constexpr auto METHOD = "method";
}

#endif /* HTTP_HEADERS */
