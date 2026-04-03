#ifndef IPARSER_HPP
#define IPARSER_HPP

class IParser {
public:
	virtual ~IParser() = default;
	virtual void parse() = 0;
};

#endif /* IPARSER_HPP */