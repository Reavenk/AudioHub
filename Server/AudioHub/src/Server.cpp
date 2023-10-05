#include "Server.h"
#include <assert.h>
#include "Utils.h"
#include "WSUser.h"
#include "Session.h"
#include "JSONUtils.h"
#include "WSHandlers/WSHException.h"
#include "WSHandlers/WSHandlerMgr.h"
#include "AudioPacket.h"
#include <mutex>

ServerLockedGuards::ServerLockedGuards(Server* server)
{
	this->server = server;
	assert(server != nullptr);
}
void ServerLockedGuards::Lock(bool users, bool sessions)
{
	std::lock_guard<std::mutex> guard(this->stateMutex);
	if (users)
	{
		assert(this->usersLocks >= 0);
		if (this->usersLocks == 0)
			this->server->_LockGuard_LockUsers();
		
		++this->usersLocks;
	}

	if (sessions)
	{
		assert(this->sessionsLocks >= 0);
		if (this->sessionsLocks == 0)
			this->server->_LockGuard_LockSessions();
		
		++this->sessionsLocks;
	}
}

void ServerLockedGuards::Unlock(bool users, bool sessions)
{
	if (users)
	{
		--this->usersLocks;
		assert(this->usersLocks >= 0);
		if(this->usersLocks == 0)
			this->server->_LockGuard_UnlockUsers();
	}

	if (sessions)
	{
		--this->sessionsLocks;
		assert(this->sessionsLocks >= 0);
		if (this->sessionsLocks == 0)
			this->server->_LockGuard_UnlockSessions();
	}
}

ServerLockedGuards::~ServerLockedGuards()
{
	assert(this->usersLocks == 0);
	assert(this->sessionsLocks == 0);
}

ServerLockedGuardsDrop::ServerLockedGuardsDrop(ServerLockedGuards* guards, bool users, bool sessions)
{
	assert(guards != nullptr);
	this->whatIsLocked = guards;

	this->users = users;
	this->sessions = sessions;

	guards->Lock(users, sessions);
}

ServerLockedGuardsDrop::~ServerLockedGuardsDrop()
{	
	this->whatIsLocked->Unlock(this->users, this->sessions);
}

Server::Server()
{
	auto& serverEP = this->server.endpoint["^/hub$"];

	// Plumbing work to redirect WS callbacks to Server.
	serverEP.on_open	= [this](WSConPtr wsCon) { this->OnWS_Opened(wsCon); };
	serverEP.on_close	= [this](WSConPtr wsCon, int code, const std::string& reason) {this->OnWS_Closed(wsCon, code, reason);};
	serverEP.on_message = [this](WSConPtr wsCon, WSInMsgPtr wsInMsg) {this->OnWS_Message(wsCon, wsInMsg); };
	serverEP.on_error	= [this](WSConPtr wsCon, const SimpleWeb::error_code& ec) {this->OnWS_Error(wsCon, ec); };
}

Server::~Server()
{
	ServerLockedGuards lockGuard(this);
	this->ShutdownServer(lockGuard);
}

bool Server::InitializeServer(int port, ServerLockedGuards& lockedGuards)
{
	// If it's a RE-initialization
	this->ShutdownServer(lockedGuards);
	assert(this->conState == ConState::Closed);
	assert(this->serverThread == nullptr);
	
	this->conState = ConState::Initializing;
	this->server.config.port = port;
	
	this->serverThread = 
		new std::thread(
		[this] 
		{
				this->server.start(
					[this] (unsigned short s) 
					{
						std::cout << "Started server" << std::endl;
						this->conState = ConState::Open;
					});
				
				this->conState = ConState::Closed;
		});
	return true;
}

bool Server::ShutdownServer(ServerLockedGuards& lockedGuards)
{
	if (this->conState == ConState::Closed)
		return false;

	if (this->conState == ConState::ShuttingDown)
	{
		// Something else is already in the process of shutting ther server down,
		// no reason to meddle.
		return false;
	}

	this->conState = ConState::ShuttingDown;
	this->server.stop();

	while (this->conState != ConState::Closed)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	this->_ShutdownSessions(lockedGuards);

	// Thread handling
	this->serverThread->join();
	delete this->serverThread;
	this->serverThread = nullptr;
	
	return true;
}

void Server::OnWS_Opened(WSConPtr wsCon)
{
	LOCKGUARD(this->userDataMutex, udm);
	ASSERT_NOTINFIND(this->unloggedConns, wsCon);
	
	// User starts as an unlogged connection.
	this->unloggedConns.insert(wsCon);
	// Send a message to the user to let them know they can login.

	json jsLogin = JSUtils::CreateHandshake(JSHS_REQLOGIN, json::object());
	wsCon->send(jsLogin.dump());
}

bool Server::ExitUserFromSession(WSUserPtr user, ServerLockedGuards& lockGuard)
{
	ServerLockedGuardsDrop guardDrop(&lockGuard, true, true);
	
	SessionPtr userSession = user->GetSession();
	SessionLockedGuards sessionGuard(userSession);
	std::cout << "Removing user " << user->Name() << "from server." << std::endl;
	
	switch (user->GetSession()->RemoveUser(user, true, sessionGuard))
	{
	case Session::RemoveResult::Removed:
		break;
	case Session::RemoveResult::RemovedNowEmpty:
	{
		// Check if it's empty again, in case lighning strikes and something
		// got added in the brief moment the mutex as disabled from Removeuser to
		// the next LOCKGUARD.
		if (userSession->Empty(sessionGuard))
		{
			userSession->Shutdown(sessionGuard);
			this->sessions.erase(userSession->SessionName());
			std::cout << "Removed session " << userSession->SessionName() << std::endl;
		}
	}
	break;
	case Session::RemoveResult::NotFound:
		return false;
		break;
	}
}

bool Server::RemoveLoggedInUser(WSConPtr wsCon, ServerLockedGuards& lockGuard)
{
	ServerLockedGuardsDrop guardDrop(&lockGuard, true, true);
	ASSERT_INFIND(this->consToUsers, wsCon);
	WSUserPtr user = this->consToUsers[wsCon];
	
	int userid = user->ID();
	bool removed = this->ExitUserFromSession(user, lockGuard);
	assert(removed || !"Error removing user from session.");

	int erased = (int)this->consToUsers.erase(wsCon);
	assert(erased == 1);
	erased = (int)this->idToUsers.erase(userid);
	assert(erased == 1);
	ASSERT_NOTINFIND(this->consToUsers, wsCon);
	ASSERT_NOTINFIND(this->idToUsers, userid);
	return true;
}

void Server::OnWS_Closed(WSConPtr wsCon, int code, const std::string& reason)
{
	ServerLockedGuards lockGuard(this);
	ServerLockedGuardsDrop guardDrop(&lockGuard, true, false);
	
	if (this->unloggedConns.find(wsCon) != this->unloggedConns.end())
	{
		std::cout << "Removing non-logged user" << std::endl;
		this->unloggedConns.erase(wsCon);
		return;
	}
	
	this->RemoveLoggedInUser(wsCon, lockGuard);
}

void Server::OnWS_Message(WSConPtr wsCon, WSInMsgPtr wsInMsg)
{	
	ServerLockedGuards lockGuard(this);
	std::string action = "_UNKNOWN";
	
	try
	{
		// If null, connection is not logged in.
		WSUserPtr user;
		{
			// Scope for the users mutex, just long enough to get user info.
			ServerLockedGuardsDrop guardDrop(&lockGuard, true, false);

			auto itFindUser = this->consToUsers.find(wsCon);
			if (itFindUser != this->consToUsers.end())
			{
				user = itFindUser->second;
			}
			else
			{
				ASSERT_INFIND(this->unloggedConns, wsCon);
			}
		}

		if (wsInMsg->fin_rsv_opcode == 130)
		{
			action = "_AudioPayload";

			// If binary payload. All binary payload is assumed to be a wav audio.
			// In the future, it would be nice to handle compressed streaming audio formats,
			// but that depends on the availability of convenient portable C++ libraries,
			// as well as C# libraries that Unity's .NET supports.

			if (user == nullptr)
				throw std::exception("Received audio for connection that isn't authorized.");

			SessionPtr session = user->GetSession();
			SessionLockedGuards sessionGuard(session);
			
			if(session == nullptr)
				throw std::exception("Received audio for connection that isn't in a session.");

			int samples = wsInMsg->size() / sizeof(short);
			if (samples != 0)
			{
				AudioPacketPtr audioPacket = std::make_shared<AudioPacket>(user, samples);
				wsInMsg->read(&audioPacket->pcmBytes[0], samples * sizeof(short));
				session->HandleAudio(audioPacket, sessionGuard);
			}
		}
		else
		{
			std::cout << "MESSAGE: " << wsInMsg->string() << std::endl;

			json jsMsg = json::parse(wsInMsg->string());
			json jsData;
			JSUtils::ExtractActionAndData(jsMsg, action, jsData);
			
			WSHandlerMgr& hmgr = WSHandlerMgr::GetInstance(); 
			WSHandlerPtr handler = hmgr.Get(action);
			if(handler == nullptr)
				throw std::exception("Invalid action.");

			// Certain handlers handle certain log-in states
			if (user == nullptr)
			{
				if(!handler->HandlesLoggedOut())
					throw std::exception("Action only supported when user is logged in.");
			}
			else
			{
				if (!handler->HandlesLoggedIn())
					throw std::exception("Action only supported when user is not logged in.");
			}

			handler->OnMessage(*this, wsCon, user, action, jsMsg, jsData, lockGuard);
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception on handler: " << e.what();

		json err = JSUtils::CreateError(action, e.what(), json::object());
		wsCon->send(err.dump());
	}
}

void Server::OnWS_Error(WSConPtr wsCon, const SimpleWeb::error_code& ec)
{
	std::cout << "Encountered error: " << ec.message() << std::endl;
}

WSUserPtr Server::GetUser(int id, ServerLockedGuards& lockGuard)
{
	ServerLockedGuardsDrop guardDrop(&lockGuard, true, false);
	
	auto it = this->idToUsers.find(id);
	if (it == this->idToUsers.end())
		return nullptr;
	
	return it->second;
}

bool Server::HasUser(int id, ServerLockedGuards& lockGuards)
{
	ServerLockedGuardsDrop guardDrop(&lockGuards, true, false);
	return this->idToUsers.find(id) != this->idToUsers.end();
}

SessionPtr Server::GetSession(const std::string& sessionName, ServerLockedGuards& lockGuard)
{
	ServerLockedGuardsDrop guardDrop(&lockGuard, false, true);
	
	auto it = this->sessions.find(sessionName);
	if (it != this->sessions.end())
		return it->second;

	SessionPtr newSession = SessionPtr(new Session(sessionName));
	this->sessions[sessionName] = newSession;
	return newSession;
}

WSUserPtr Server::ConvertToUser(
	WSConPtr conToUpgrade, 
	const std::string& username, 
	const std::string& sessionName, 
	const std::string& action,
	ServerLockedGuards& lockGuards)
{
	ServerLockedGuardsDrop guardDrop(&lockGuards, true, true);
	
	SessionPtr session = this->GetSession(sessionName, lockGuards);
	SessionLockedGuards sessionGuard(session);
	
	WSUserPtr user = WSUserPtr(new WSUser(conToUpgrade, username, this->NewID(), session));
	this->unloggedConns.erase(conToUpgrade);
	this->consToUsers[conToUpgrade] = user;
	this->idToUsers[user->ID()] = user;
	session->AddUser(user, action, sessionGuard);
	
	return user;
}

bool Server::LogoutUser(WSUserPtr user, ServerLockedGuards& lockGuards)
{
	ServerLockedGuardsDrop guardDrop(&lockGuards, true, true);

	WSConPtr con = user->GetWSCon();
	if (con == nullptr)
		return false;

	this->RemoveLoggedInUser(con, lockGuards);
	ASSERT_NOTINFIND(this->unloggedConns, con);
	this->unloggedConns.insert(con);
	ASSERT_IN(this->unloggedConns, con);

	return true;
}

void Server::_ShutdownSessions(ServerLockedGuards& lockedGuards)
{
	ServerLockedGuardsDrop guardDrop(&lockedGuards, false, true);
	
	for (auto it : this->sessions)
	{
		SessionLockedGuards sessionGuard(it.second);
		it.second->Shutdown(sessionGuard);
	}

	this->sessions.clear();
}

void Server::_LockGuard_LockUsers()
{
	this->userDataMutex.lock();
}

void Server::_LockGuard_UnlockUsers()
{
	this->userDataMutex.unlock();
}

void Server::_LockGuard_LockSessions()
{
	this->sessionContainersMutex.lock();
}

void Server::_LockGuard_UnlockSessions()
{
	this->sessionContainersMutex.unlock();
}

UIDTy Server::NewID()
{
	return this->idCounter++;
}