#pragma once

enum class BodyState
{
	Incomplete,
	Complete,
	Overflow,
	Chunked,
	CGI,
	Invalid
};
