#include "HttpStatus.hpp"

// Map code to standard name
std::string HttpStatus::get_status_code_name(e_code s)
{
    switch (s)
    {
        // 1xx Informational
        case e_code::CONTINUE: return "Continue";
        case e_code::SWITCHING_PROTOCOLS: return "Switching Protocols";
        case e_code::PROCESSING: return "Processing";
        case e_code::EARLY_HINTS: return "Early Hints";

        // 2xx Success
        case e_code::OK: return "OK";
        case e_code::CREATED: return "Created";
        case e_code::ACCEPTED: return "Accepted";
        case e_code::NON_AUTHORITATIVE_INFORMATION: return "Non-Authoritative Information";
        case e_code::NO_CONTENT: return "No Content";
        case e_code::RESET_CONTENT: return "Reset Content";
        case e_code::PARTIAL_CONTENT: return "Partial Content";
        case e_code::MULTI_STATUS: return "Multi-Status";
        case e_code::ALREADY_REPORTED: return "Already Reported";
        case e_code::IM_USED: return "IM Used";

        // 3xx Redirection
        case e_code::MULTIPLE_CHOICES: return "Multiple Choices";
        case e_code::MOVED_PERMANENTLY: return "Moved Permanently";
        case e_code::FOUND: return "Found";
        case e_code::SEE_OTHER: return "See Other";
        case e_code::NOT_MODIFIED: return "Not Modified";
        case e_code::USE_PROXY: return "Use Proxy";
        case e_code::TEMPORARY_REDIRECT: return "Temporary Redirect";
        case e_code::PERMANENT_REDIRECT: return "Permanent Redirect";

        // 4xx Client errors
        case e_code::BAD_REQUEST: return "Bad Request";
        case e_code::UNAUTHORIZED: return "Unauthorized";
        case e_code::PAYMENT_REQUIRED: return "Payment Required";
        case e_code::FORBIDDEN: return "Forbidden";
        case e_code::NOT_FOUND: return "Not Found";
        case e_code::METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case e_code::NOT_ACCEPTABLE: return "Not Acceptable";
        case e_code::PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
        case e_code::REQUEST_TIMEOUT: return "Request Timeout";
        case e_code::CONFLICT: return "Conflict";
        case e_code::GONE: return "Gone";
        case e_code::LENGTH_REQUIRED: return "Length Required";
        case e_code::PRECONDITION_FAILED: return "Precondition Failed";
        case e_code::PAYLOAD_TOO_LARGE: return "Payload Too Large";
        case e_code::URI_TOO_LONG: return "URI Too Long";
        case e_code::UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
        case e_code::RANGE_NOT_SATISFIABLE: return "Range Not Satisfiable";
        case e_code::EXPECTATION_FAILED: return "Expectation Failed";
        case e_code::IM_A_TEAPOT: return "I'm a teapot";
        case e_code::MISDIRECTED_REQUEST: return "Misdirected Request";
        case e_code::UNPROCESSABLE_ENTITY: return "Unprocessable Entity";
        case e_code::LOCKED: return "Locked";
        case e_code::FAILED_DEPENDENCY: return "Failed Dependency";
        case e_code::TOO_EARLY: return "Too Early";
        case e_code::UPGRADE_REQUIRED: return "Upgrade Required";
        case e_code::PRECONDITION_REQUIRED: return "Precondition Required";
        case e_code::TOO_MANY_REQUESTS: return "Too Many Requests";
        case e_code::REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";
        case e_code::UNAVAILABLE_FOR_LEGAL_REASONS: return "Unavailable For Legal Reasons";

        // 5xx Server errors
        case e_code::INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case e_code::NOT_IMPLEMENTED: return "Not Implemented";
        case e_code::BAD_GATEWAY: return "Bad Gateway";
        case e_code::SERVICE_UNAVAILABLE: return "Service Unavailable";
        case e_code::GATEWAY_TIMEOUT: return "Gateway Timeout";
        case e_code::HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
        case e_code::VARIANT_ALSO_NEGOTIATES: return "Variant Also Negotiates";
        case e_code::INSUFFICIENT_STORAGE: return "Insufficient Storage";
        case e_code::LOOP_DETECTED: return "Loop Detected";
        case e_code::NOT_EXTENDED: return "Not Extended";
        case e_code::NETWORK_AUTHENTICATION_REQUIRED: return "Network Authentication Required";

        case e_code::UNKNOWN: 
        default: return "Unknown";
    }
}

// Generic messages
std::string HttpStatus::get_message(e_code code) {
    int num = static_cast<int>(code);
    if (num >= 100 && num < 200) return "Informational response";
    if (num >= 200 && num < 300) return "Request successful";
    if (num >= 300 && num < 400) return "Redirection message";
    if (num >= 400 && num < 500) return "Client error";
    if (num >= 500 && num < 600) return "Server error";
    return "Unknown status";
}

// Stream output
std::ostream& operator<<(std::ostream& os, HttpStatus::e_code code)
{
    return os << static_cast<int>(code) << " " << HttpStatus::get_status_code_name(code);
}

// Conversions
unsigned int HttpStatus::number_from_code(e_code code)
{
    return static_cast<unsigned int>(code);
}

HttpStatus::e_code HttpStatus::code_from_number(unsigned int code)
{
    return e_code(code);
}

// Status type checks
bool HttpStatus::is_bad(e_code code)
{
    int n = static_cast<int>(code);
    return n >= 400 && n < 600;
}

bool HttpStatus::is_redirect(e_code code)
{
    int n = static_cast<int>(code);
    return n >= 300 && n < 400;
}

bool HttpStatus::is_good(e_code code)
{
    int n = static_cast<int>(code);
    return n >= 200 && n < 300;
}