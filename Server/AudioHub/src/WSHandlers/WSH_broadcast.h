#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS Message handler for a user to broadcast a JSON payload message
/// to all users in the same char session. 
/// </summary>
class WSH_broadcast : public IWSHandler
{
public:
	WSH_broadcast();
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