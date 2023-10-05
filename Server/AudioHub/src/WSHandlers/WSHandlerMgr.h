#pragma once

#include "IWSHandler.h"

/// <summary>
/// 
/// </summary>
class WSHandlerMgr
{
private:
	std::map<std::string, WSHandlerPtr> registered;

private:
	WSHandlerMgr();

public:
	~WSHandlerMgr();
	WSHandlerMgr& operator= (const WSHandlerMgr&) = delete;
	WSHandlerMgr(const WSHandlerMgr&) = delete;

	bool Register(WSHandlerPtr handler);

	WSHandlerPtr operator[](const std::string& action);
	WSHandlerPtr Get(const std::string& action);


public:
	static WSHandlerMgr& GetInstance();
};

/// <summary>
/// 
/// </summary>
class _WSHRegUtil
{
public:
	_WSHRegUtil(WSHandlerPtr toReg);
};

#define REGISTER_WSH( cl ) \
	static _WSHRegUtil _globalReg(WSHandlerPtr(new cl())); 
