#pragma once

enum class HeaderState
{
	Complete,
	Incomplete,
	ContainsBody,
	Error,
	Redirect,
	CGI
};
