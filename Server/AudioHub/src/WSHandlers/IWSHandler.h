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

	virtual bool HandlesLoggedIn() = 0;
	virtual bool HandlesLoggedOut() = 0;

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