#ifndef LISTEN_DATA_HPP
#define LISTEN_DATA_HPP

#include <string>
#include <ostream>

struct ListenData
{
	short		port;
	std::string	ip_address;

	bool operator==(const ListenData& other) const {
		return port == other.port && ip_address == other.ip_address;
	}
};

struct ListenDataHash {
	std::size_t operator()(const ListenData& loc) const {
		return std::hash<int>()(loc.port) ^ 
			   (std::hash<std::string>()(loc.ip_address) << 1);
	}
};

inline std::ostream& operator<<(std::ostream& os, const ListenData& ld)
{
	os << "ListenData { ip: " << ld.ip_address << ", port: " << ld.port << " }";
	return os;
}

#endif /* LISTEN_DATA_HPP */
