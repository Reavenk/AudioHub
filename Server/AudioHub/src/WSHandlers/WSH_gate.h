#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS message handler for a user to set the gate (opposite of mute)
/// setting for another player.
/// </summary>
class WSH_gate : public IWSHandler
{
public:
	WSH_gate();
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