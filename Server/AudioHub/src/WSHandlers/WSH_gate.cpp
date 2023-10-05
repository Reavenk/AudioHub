#include "WSH_gate.h"
#include "WSHandlerMgr.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"
#include "WSHException.h"

WSH_gate::WSH_gate()
{}

std::string WSH_gate::ActionName()
{
	return "gate";
}

bool WSH_gate::HandlesLoggedIn()
{
	return true;
}

bool WSH_gate::HandlesLoggedOut()
{
	return false;
}

void WSH_gate::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	bool gate = JSUtils::ExtractExpectedBool("gate", data);
	int userid = JSUtils::ExtractExpectedInt("userid", data);

	WSUserPtr userToSetVol = server.GetUser(userid, lockGuard);
	if (userToSetVol == nullptr)
		throw WSHException(action, "User id is not valid");

	if(userToSetVol == user)
		throw WSHException(action, "Cannot set gate for self");

	SessionPtr userSession = user->GetSession();
	SessionLockedGuards sessionGuard(userSession);
	if(!userSession->HasUser(userToSetVol, sessionGuard))
		throw WSHException(action, "User id is not valid");
	
	user->SetGate_LOCK(userToSetVol, gate);

	json gateData;
	gateData["gate"] = gate;
	gateData["user"] = GenerateUserBroadcastEntry(userToSetVol);
	json jsResp = JSUtils::CreateResponse(JSREQ_GATE, gateData);
	con->send(jsResp.dump());
}

REGISTER_WSH(WSH_gate);