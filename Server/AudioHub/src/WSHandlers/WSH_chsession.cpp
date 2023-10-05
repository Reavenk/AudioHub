#include "WSH_chsession.h"
#include "WSHandlerMgr.h"
#include "../JSONUtils.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"
#include "WSHException.h"

WSH_chsession::WSH_chsession()
{}

std::string WSH_chsession::ActionName()
{
	return "chsession";
}

bool WSH_chsession::HandlesLoggedIn()
{
	return true;
}

bool WSH_chsession::HandlesLoggedOut()
{
	return false;
}

void WSH_chsession::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	// Get requested session
	std::string sessionNameToChange = JSUtils::ExtractExpectedString("session", data);
	if (sessionNameToChange.empty())
		throw WSHException(action, "Missing session name.");
	
	SessionPtr oldSession = user->GetSession();
	if(oldSession->SessionName() == sessionNameToChange)
		throw WSHException(action, "User is already in that session");
	
	// Make sure nothing happens to transferSession after we get it.
	ServerLockedGuardsDrop guardDrop(&lockGuard, true, true); 
	//
	SessionPtr transferSession = server.GetSession(sessionNameToChange, lockGuard);
	SessionLockedGuards sessionGuard(transferSession);
	
	if(!server.ExitUserFromSession(user, lockGuard))
		throw WSHException(action, "Failed to exit user from session");
	
	// AddUser() handles sending back the response
	user->_ResetSession(transferSession);
	transferSession->AddUser(user, JSREQ_CHSESSION, sessionGuard);
	std::cout << "Transferred user " << user->Name() << " to session " << sessionNameToChange << std::endl;
}

REGISTER_WSH(WSH_chsession);