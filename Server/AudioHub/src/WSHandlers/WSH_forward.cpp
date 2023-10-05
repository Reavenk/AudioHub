#include "WSH_forward.h"
#include "WSHandlerMgr.h"
#include "../JSONUtils.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"
#include "WSHException.h"

WSH_forward::WSH_forward()
{}

std::string WSH_forward::ActionName()
{
	return "forward";
}

bool WSH_forward::HandlesLoggedIn()
{
	return true;
}

bool WSH_forward::HandlesLoggedOut()
{
	return false;
}

void WSH_forward::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	int recipient = JSUtils::ExtractExpectedInt("recipid", data);

	WSUserPtr userRecipeint = server.GetUser(recipient, lockGuard);
	if (userRecipeint == nullptr)
		throw WSHException(action, "Recipient id is not valid");

	SessionPtr userSession = user->GetSession();
	SessionLockedGuards sessionGuard(userSession);
	if(!userSession->HasUser(userRecipeint, sessionGuard))
		throw WSHException(action, "Recipient id is not valid");
	
	json dataCpy = data;
	dataCpy["senderid"] = user->ID();

	json jsResp = JSUtils::CreateEvent(JSNOTIF_FORWARD, dataCpy);
	userRecipeint->GetWSCon()->send(jsResp.dump());
}

REGISTER_WSH(WSH_forward);