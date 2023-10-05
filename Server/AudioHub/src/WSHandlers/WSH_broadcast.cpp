#include "WSH_broadcast.h"
#include "WSHandlerMgr.h"
#include "../JSONUtils.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"

WSH_broadcast::WSH_broadcast()
{}

std::string WSH_broadcast::ActionName()
{
	return "broadcast";
}

bool WSH_broadcast::HandlesLoggedIn()
{
	return true;
}

bool WSH_broadcast::HandlesLoggedOut()
{
	return false;
}

void WSH_broadcast::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	json dataCpy = data;
	dataCpy["senderid"] = user->ID();

	json jsResp = JSUtils::CreateResponse(JSNOTIF_FORWARD, dataCpy);
	
	SessionPtr session = user->GetSession();
	SessionLockedGuards sessionGuard(session);
	session->Broadcast(jsResp.dump(), user, sessionGuard);
}

REGISTER_WSH(WSH_broadcast);