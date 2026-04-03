#include "ListenData.hpp"

bool ListenData::operator==(const ListenData& other) const {
	return port == other.port && ip_address == other.ip_address;
}

std::size_t ListenDataHash::operator()(const ListenData& loc) const {
	return std::hash<int>()(loc.port) ^ (std::hash<std::string>()(loc.ip_address) << 1);
}

std::ostream& operator<<(std::ostream& os, const ListenData& ld)
{
	os << "ListenData { ip: " << ld.ip_address << ", port: " << ld.port << " }";
	return os;
}