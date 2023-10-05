#include "WSH_login.h"
#include "WSHandlerMgr.h"
#include "../Server.h"
#include "../JSONUtils.h"
#include "../WSUser.h"
#include "../Session.h"

WSH_login::WSH_login()
	: IWSHandler()
{}

std::string WSH_login::ActionName()
{
	return "login";
}

bool WSH_login::HandlesLoggedIn()
{
	return false;
}

bool WSH_login::HandlesLoggedOut()
{
	return true;
}
		 
void WSH_login::OnMessage(
	Server& server,
	WSConPtr con,
	WSUserPtr user,
	std::string& action,
	const json& fullMsg,
	const json& data,
	ServerLockedGuards& lockGuard)
{
	std::string username = JSUtils::ExtractExpectedString("username", data);
	std::string session = JSUtils::ExtractExpectedString("session", data);
	
	WSUserPtr newuser = server.ConvertToUser(con, username, session, JSREQ_LOGIN, lockGuard);
	std::cout << "Logged in user " << newuser << " to session " << session << std::endl;
}

REGISTER_WSH(WSH_login);