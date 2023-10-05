#pragma once

#include "IWSHandler.h"

/// <summary>
/// Manager to process WS JSON message.
/// </summary>
class WSHandlerMgr
{
private:
	/// <summary>
	/// The WS message handlers supported by the manager.
	/// This is populated through the use of Register().
	/// </summary>
	std::map<std::string, WSHandlerPtr> registered;

private:
	WSHandlerMgr();

public:
	~WSHandlerMgr();
	WSHandlerMgr& operator= (const WSHandlerMgr&) = delete;
	WSHandlerMgr(const WSHandlerMgr&) = delete;

	/// <summary>
	/// Dependency inject a WS message handler to support.
	/// 
	/// This does not have to be done manually. Most of the time
	/// this will be done automatically with the use of the
	/// REGISTER_WSH() macro.
	/// </summary>
	bool Register(WSHandlerPtr handler);

	WSHandlerPtr operator[](const std::string& action);
	WSHandlerPtr Get(const std::string& action);


public:
	/// <summary>
	/// Singleton getter.
	/// </summary>
	static WSHandlerMgr& GetInstance();
};

/// <summary>
/// Class to automatically register WS handlers to the WSHandlerMgr singleton.
/// </summary>
class _WSHRegUtil
{
public:
	_WSHRegUtil(WSHandlerPtr toReg);
};

// Macro to automatically register
#define REGISTER_WSH( cl ) \
	static _WSHRegUtil _globalReg(WSHandlerPtr(new cl())); 
