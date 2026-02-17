#include "HttpResponseWriter.hpp"

HttpResponseWriter::HttpResponseWriter(/* args */)
{
}

HttpResponseWriter::~HttpResponseWriter()
{
}

void HttpResponseWriter::formResponse()
{
	if (_response_formed)
		return;

	if (_cgi_handler)
	{
		if (!_cgi_handler->isResponseReady())
			return;

		CGIExitStatus	status = _cgi_handler->getExitStatus();

		std::cout << "CGI exit status: " << (status == CGIExitStatus::SUCCESS ? "Success" : "Error") << std::endl;

		if (status == CGIExitStatus::SUCCESS)
			_response.form_response(_request.get_status_code(), _request.copy_headers(), _cgi_handler->getBuffer());
		else
			_response.form_response(_request.get_status_code(), _request.copy_headers());

		_cgi_handler.reset();
	}
	else
		_response.form_response(_request.get_status_code(), _request.copy_headers());

	_removeBodyFromBuffer();
	_response_formed = true;
}

