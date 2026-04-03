#ifndef LISTEN_DATA_HPP
#define LISTEN_DATA_HPP

#include <string>
#include <ostream>

struct ListenData
{
	int			port;
	std::string	ip_address;

	bool operator==(const ListenData& other) const;
};

struct ListenDataHash {
	std::size_t operator()(const ListenData& loc) const;
};

std::ostream& operator<<(std::ostream& os, const ListenData& ld);

#endif /* LISTEN_DATA_HPP */
