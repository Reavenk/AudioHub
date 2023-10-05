#include "WSHException.h"

WSHException::WSHException(const std::string& action, const std::string& what_val)
	: std::exception(what_val.c_str())
{
	this->action = action;
	this->what_val = what_val;
}

char const* WSHException::what() const
{
	return this->what_val.c_str();
}