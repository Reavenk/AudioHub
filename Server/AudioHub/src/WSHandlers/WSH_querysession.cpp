#include "WSH_querysession.h"
#include "WSHandlerMgr.h"

WSH_querysession::WSH_querysession()
{}

std::string WSH_querysession::ActionName()
{
	return "querysession";
}

bool WSH_querysession::HandlesLoggedIn()
{
	return false;
}

bool WSH_querysession::HandlesLoggedOut()
{
	return true;
}

void WSH_querysession::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	//NOTE: Currently unused
	// But this message would give the client back
	// info on their connection and session
}

REGISTER_WSH(WSH_querysession);