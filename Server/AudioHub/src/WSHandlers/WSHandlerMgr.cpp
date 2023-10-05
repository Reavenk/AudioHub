#include "WSHandlerMgr.h"

WSHandlerMgr::WSHandlerMgr()
{}

WSHandlerMgr::~WSHandlerMgr()
{}

bool WSHandlerMgr::Register(WSHandlerPtr handler)
{
	std::string action = handler->ActionName();
	if (this->registered.find(action) != this->registered.end())
		return false;

	this->registered[action] = handler;
	return true;
}

WSHandlerPtr WSHandlerMgr::operator[](const std::string& action)
{
	return this->Get(action);
}

WSHandlerPtr WSHandlerMgr::Get(const std::string& action)
{
	auto it = this->registered.find(action);
	if (it == this->registered.end())
		return nullptr;

	return it->second;
}

WSHandlerMgr& WSHandlerMgr::GetInstance()
{
	static WSHandlerMgr mgr;
	return mgr;
}

_WSHRegUtil::_WSHRegUtil(WSHandlerPtr toReg)
{
	WSHandlerMgr& mgr = WSHandlerMgr::GetInstance();
	mgr.Register(toReg);
}