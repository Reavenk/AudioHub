#include "WSH_rebuffer.h"
#include "WSHandlerMgr.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"
#include "WSHException.h"

WSH_rebuffer::WSH_rebuffer()
	: IWSHandler()
{}

std::string WSH_rebuffer::ActionName()
{
	return "rebuffer";
}

bool WSH_rebuffer::HandlesLoggedIn()
{
	return true;
}

bool WSH_rebuffer::HandlesLoggedOut()
{
	return false;
}
		 
void WSH_rebuffer::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	const json* jsToRebuf = JSUtils::ExtractJSON("users", data);

	int rebufferSamples = SAMPLERATE / 2;
	JSUtils::ExtractInt("samples", data, rebufferSamples);

	if (jsToRebuf != nullptr && jsToRebuf->is_array())
		throw WSHException(action, "Userlist must be an array of user ids");

	std::vector<int> userIDs;
	if (jsToRebuf != nullptr)
	{
		for (const json& js : *jsToRebuf)
		{
			if (!js.is_number())
				continue;

			userIDs.push_back((int)js);
		}
	}

	user->RebufferStreams_LOCK(userIDs, rebufferSamples);
	json jsResp = JSUtils::CreateResponse(JSNOTIF_REBUFFER, json::object());
	con->send(jsResp.dump());
}

REGISTER_WSH(WSH_rebuffer);