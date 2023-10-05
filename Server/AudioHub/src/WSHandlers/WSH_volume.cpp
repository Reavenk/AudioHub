#include "WSH_volume.h"
#include "WSHandlerMgr.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"
#include "WSHException.h"

WSH_volume::WSH_volume()
{}

std::string WSH_volume::ActionName()
{
	return "volume";
}

bool WSH_volume::HandlesLoggedIn()
{
	return true;
}

bool WSH_volume::HandlesLoggedOut()
{
	return false;
}

void WSH_volume::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	float volume = JSUtils::ExtractExpectedFloat("volume", data);
	int userid = JSUtils::ExtractExpectedInt("userid", data);

	volume = std::min(1.0f, std::max(0.0f, volume));

	WSUserPtr userToSetVol = server.GetUser(userid, lockGuard);
	if (userToSetVol == nullptr)
		throw WSHException(action, "User id is not valid");

	if(userToSetVol == user)
		throw WSHException(action, "Cannot set volume for self");

	SessionPtr userSession = user->GetSession();
	SessionLockedGuards sessionGuard(userSession);
	if(!userSession->HasUser(userToSetVol, sessionGuard))
		throw WSHException(action, "User id is not valid");
	
	user->SetVolume_LOCK(userToSetVol, volume);

	json volumeData;
	volumeData["volume"] = volume;
	volumeData["user"] = GenerateUserBroadcastEntry(userToSetVol);
	json jsResp = JSUtils::CreateResponse(JSREQ_VOLUME, volumeData);
	con->send(jsResp.dump());
}

REGISTER_WSH(WSH_volume);