#include "HttpMethod.hpp"

HttpMethod::e_code HttpMethod::fromString(const std::string& method) {
	if (method == "OPTIONS") return e_code::OPTIONS;
	if (method == "GET") return e_code::GET;
	if (method == "POST") return e_code::POST;
	if (method == "DELETE") return e_code::DELETE;
	return e_code::INVALID;
}

std::string HttpMethod::toString(e_code code) {
	switch (code) {
		case e_code::OPTIONS: return "OPTIONS";
		case e_code::GET: return "GET";
		case e_code::POST: return "POST";
		case e_code::DELETE: return "DELETE";
	}
	return "INVALID";
}