#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS message handler for a user to requesting changing the chat session
/// they're participating in.
/// </summary>
class WSH_chsession : public IWSHandler
{
public:
	WSH_chsession();
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