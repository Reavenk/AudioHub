#pragma once
#include "IWSHandler.h"

class WSH_login : public IWSHandler
{
public:
	WSH_login();
	std::string ActionName() override;

	bool HandlesLoggedIn() override;
	bool HandlesLoggedOut() override;

	void OnMessage(
		Server& server,
		WSConPtr con, 
		WSUserPtr user, 
		std::string& action, 
		const json& fullMsg, 
		const json& data,
		ServerLockedGuards& lockGuard) override;
};