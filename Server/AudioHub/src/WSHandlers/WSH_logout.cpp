#include "WSH_logout.h"
#include "WSHandlerMgr.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"
#include "WSHException.h"

WSH_logout::WSH_logout()
{}

std::string WSH_logout::ActionName()
{
	return "logout";
}

bool WSH_logout::HandlesLoggedIn()
{
	return true;
}

bool WSH_logout::HandlesLoggedOut()
{
	return false;
}

void WSH_logout::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	std::cout << "Logging out user " << user->Name() << std::endl;
	bool loggedOut = server.LogoutUser(user, lockGuard);
	if (!loggedOut)
		throw WSHException(action, "Could not log out user");

	json jsResp = JSUtils::CreateResponse(JSREQ_LOGOUT, json::object());
	con->send(jsResp.dump());
}

REGISTER_WSH(WSH_logout);