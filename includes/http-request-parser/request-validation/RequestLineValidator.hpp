#ifndef REQUEST_LINE_VALIDATOR
#define REQUEST_LINE_VALIDATOR

#include <string>

class RequestLineValidator
{
private:
	std::string	_upload_dir;
	uint		_uploaded_files_count;

public:
	RequestLineValidator(/* args */);
	~RequestLineValidator();
};

#endif REQUEST_LINE_VALIDATOR
