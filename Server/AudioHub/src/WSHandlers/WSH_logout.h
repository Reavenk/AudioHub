#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS message handler for a user to log out of the server.
/// </summary>
class WSH_logout : public IWSHandler
{
public:
	WSH_logout();
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