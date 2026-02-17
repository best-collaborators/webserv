#pragma once

enum class HeaderState
{
	Complete,
	Incomplete,
	ContainsBody,
	Bad
};
