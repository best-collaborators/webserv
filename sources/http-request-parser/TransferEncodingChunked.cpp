#include "TransferEncodingChunkedParser.hpp"

TransferEncodingChunkedParser::TransferEncodingChunkedParser( std::string &raw_bits, Request &request ) :
_request(request), _raw_bits(raw_bits) { }

void TransferEncodingChunkedParser::parse()
{
	std::string buffer = _request.get_current_chunk();

	std::string temp = ValidatorHelpers::cut_after_new_line(_raw_bits);
	if (temp.empty() && !_raw_bits.empty()) {
		temp += std::move(_raw_bits);
		_raw_bits = "";
	}
	buffer += temp;

	// std::cout << "RAW BITS: " << std::endl << _raw_bits << std::endl;
	// std::cout << "TEMP: " << std::endl << temp << std::endl;
	std::cout << "BUFFER: " << std::endl << buffer << std::endl;
	std::cout << "_request.get_current_chunk_size(): " << _request.get_current_chunk_size() << std::endl;
	std::cout << "_request.get_current_chunk_size_actual(): " << _request.get_current_chunk_size_actual() << std::endl;

	while (!buffer.empty())
	{
		unsigned long long chunk_size = 0;
		if (!_request.get_current_chunk_size()) {
			std::string hex = RegexMatcher::get_regex_value(buffer, HttpRegexPatterns::HEX_VALUE());
			try {
				chunk_size = std::stoull(hex, nullptr, 16);
			}
			catch(const std::exception& e) { 
				std::cout << "[http-parser] Invalid size in transfer-encoding --> " << hex << std::endl; 
				_request.set_status_code(400);
				_request.set_is_chunk_received(true);
				_request.set_current_chunk("");
				_request.set_chunk_size(0);
				return ;
			}
			_request.set_chunk_size(chunk_size);

			buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);
			if (buffer.empty() && !_raw_bits.empty()) {
				buffer = std::move(_raw_bits);
				_raw_bits = "";
			}
		}
		else {
			chunk_size = _request.get_current_chunk_size();
		}

		if (chunk_size == 0 && buffer.empty() && _raw_bits.empty()) {
			_request.set_status_code(200);
			_request.set_is_chunk_received(true);
			return ;
		}

		if ((chunk_size > 0 && buffer.empty())) {
			std::cout << "[http-parser] Invalid chunk in transfer-encoding" << std::endl; 
			_request.set_status_code(400);
			return ;
		}

		if (buffer.size() == chunk_size) {
			_request.append_body_value(buffer);
			_request.set_current_chunk("");
			_request.set_chunk_size(0);
			buffer = ValidatorHelpers::cut_after_new_line(_raw_bits);
			continue ;
		}

		while (buffer.size() >= chunk_size && !_raw_bits.empty()) {
			std::cout << "buffer size: " << buffer.size() << std::endl;
			std::string temp = ValidatorHelpers::cut_after_new_line(_raw_bits);
			if (temp.empty() && !_raw_bits.empty()) {
				temp += std::move(_raw_bits);
				_raw_bits = "";
			}
			buffer += temp;
		}
		if (buffer.size() >= chunk_size)
			_request.append_body_value(buffer);
		_request.set_current_chunk(std::move(buffer));
		if (_raw_bits.empty()) break;
	}
}