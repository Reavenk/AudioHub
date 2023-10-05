#pragma once

#include "../Types.h"
#include "../SimpleWSinclude.h"
#include <string>
#include "../vendored/nlohmann/json.hpp"
#include <memory>

using json = nlohmann::json;

class IWSHandler
{
public:
	IWSHandler();
	virtual ~IWSHandler();
	IWSHandler& operator=(const IWSHandler&) = delete;
	IWSHandler(const IWSHandler&) = delete;

	virtual std::string ActionName() = 0;

	/// <summary>
	/// Is the handler available for messages from connections that are 
	/// logged in?
	/// </summary>
	virtual bool HandlesLoggedIn() = 0;

	/// <summary>
	/// Is the handler available for message from connections that are
	/// not logged in?
	/// </summary>
	/// <returns></returns>
	virtual bool HandlesLoggedOut() = 0;

	/// <summary>
	/// General message handler.
	/// </summary>
	/// <param name="server">Access to the server object.</param>
	/// <param name="con">The connection sending the message.</param>
	/// <param name="user">The logged in user info (if connection is logged in)</param>
	/// <param name="action">The action, extracted from the JSON payload.</param>
	/// <param name="fullMsg">The JSON payload.</param>
	/// <param name="data">The JSON data section.</param>
	/// <param name="lockGuard">Server mutual exclusion guard.</param>
	virtual void OnMessage(
		Server& server,
		WSConPtr con, 
		WSUserPtr user, 
		std::string& action, 
		const json& fullMsg, 
		const json& data,
		ServerLockedGuards& lockGuard) = 0;
};

typedef std::shared_ptr<IWSHandler> WSHandlerPtr;