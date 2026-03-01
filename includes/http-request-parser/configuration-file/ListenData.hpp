#ifndef LISTEN_DATA_HPP
#define LISTEN_DATA_HPP

#include <string>
#include <ostream>

struct ListenData
{
	short		port;
	std::string	ip_address;
};

inline std::ostream& operator<<(std::ostream& os, const ListenData& ld)
{
	os << "ListenData { ip: " << ld.ip_address << ", port: " << ld.port << " }";
	return os;
}

#endif /* LISTEN_DATA_HPP */
