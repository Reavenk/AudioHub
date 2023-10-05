#pragma once
#include "IWSHandler.h"

/// <summary>
/// WS message handler to query the server for information
/// on their connection and session.
/// </summary>
class WSH_querysession : public IWSHandler
{
public:
	WSH_querysession();
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