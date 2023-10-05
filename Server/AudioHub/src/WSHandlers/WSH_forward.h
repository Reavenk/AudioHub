#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS message handler for a user to send a JSON payload to
/// a specific user.
/// </summary>
class WSH_forward : public IWSHandler
{
public:
	WSH_forward();
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