#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS message handler for a user to change the mixing volume of
/// a specific user.
/// </summary>
class WSH_volume : public IWSHandler
{
public:
	WSH_volume();
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