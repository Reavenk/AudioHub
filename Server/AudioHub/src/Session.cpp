#include "Session.h"
#include "WSUser.h"
#include <assert.h>
#include "Utils.h"
#include "JSONUtils.h"
#include "AudioPacket.h"

#include <chrono>

void SessionLockedGuards::Lock(bool users)
{
	if (users)
	{
		assert(this->usersLocks >= 0);
		if(this->usersLocks == 0)
			stateMutex.lock();

		++this->usersLocks;
	}

}

void SessionLockedGuards::Unlock(bool users)
{
	if (users)
	{
		assert(this->usersLocks > 0);
		--this->usersLocks;
		
		if(this->usersLocks == 0)
			stateMutex.unlock();
	}
}

SessionLockedGuards::SessionLockedGuards(SessionPtr session)
{
	this->session = session.get();
}

SessionLockedGuards::SessionLockedGuards(Session* session)
{
	this->session = session;
}

SessionLockedGuards::~SessionLockedGuards()
{
	assert(this->usersLocks == 0);
}

SessionLockedGuardsDrop::SessionLockedGuardsDrop(SessionLockedGuards* whatIsLocked, bool users)
{
	this->whatIsLocked = whatIsLocked;
	this->users = users;
	this->whatIsLocked->Lock(this->users);
}

SessionLockedGuardsDrop::~SessionLockedGuardsDrop()
{
	this->whatIsLocked->Unlock(this->users);
}

json GenerateUserBroadcastEntry(WSUserPtr user)
{
	json ret = json::object();
	ret["id"] = user->ID();
	ret["name"] = user->Name();
	return ret;
}

Session::Session(const std::string& sessionName)
{
	this->sessionName = sessionName;
	this->serviceThread = 
		std::thread(
			[this]
			{ 
				this->ThreadFn(); 
				this->runningState = RunningState::Dead;
			});
}

Session::~Session()
{
	SessionLockedGuards lockGuards(this);
		
	if (this->runningState == RunningState::Alive)
		this->Shutdown(lockGuards);
}

void Session::AddUser(WSUserPtr user, const std::string& action, SessionLockedGuards& lockGuards)
{
	ASSERT_NOTINFIND(this->userDirectory, user);
	// AddUser should only be called to users who have already been
	// prepped for adding to this session.
	assert(user->SessionName() == this->sessionName);

	this->userDirectory.insert(user);
	ASSERT_INFIND(this->userDirectory, user);
	
	// Send information to the user about the session they logged into as
	// part of the response.
	std::vector<WSUserPtr> usersCpy = user->GetSession()->GetUsersCopy(lockGuards);
	json loginData;
	loginData["username"] = user->Name();
	loginData["session"] = this->SessionName();
	json users = json::array();
	for (WSUserPtr u : usersCpy)
		users.push_back(GenerateUserBroadcastEntry(u));
	loginData["users"] = users;
	loginData["selfid"] = user->ID();

	json jsResp = JSUtils::CreateResponse(action, loginData);
	user->GetWSCon()->send(jsResp.dump());

	// To all other users, send notice of new user
	json newUserData = json::object();
	json newUserList = json::array();
	newUserList.push_back(GenerateUserBroadcastEntry(user));
	newUserData["enter"] = newUserList;
	json jsNotifNewUser = JSUtils::CreateEvent(JSNOTIF_USERENTER, newUserData);
	this->Broadcast(jsNotifNewUser.dump(), user, lockGuards);
}

bool Session::HasUser(WSUserPtr user, SessionLockedGuards& lockGuards)
{
	SessionLockedGuardsDrop guardDrop(&lockGuards, true);
	return this->userDirectory.find(user) != this->userDirectory.end();
}


bool Session::Empty(SessionLockedGuards& lockGuards) const
{
	SessionLockedGuardsDrop guardDrop(&lockGuards, true);
	return this->userDirectory.empty();
}

Session::RemoveResult Session::RemoveUser(WSUserPtr user, bool broadcast, SessionLockedGuards& lockGuards)
{
	SessionLockedGuardsDrop guardDrop(&lockGuards, true);

	ASSERT_IN(this->userDirectory, user);

	// Yes, there's an assert, but a little extra runtime release-build saftey, plus
	// we have an error case to code in for a return value.
	if (this->userDirectory.find(user) == this->userDirectory.end())
	{
		std::cerr << "Attempting to remove user " << user->Name() << " from session " << this->sessionName << " but not found." << std::endl;
		return RemoveResult::NotFound;
	}
	
	int erased = (int)this->userDirectory.erase(user);
	assert(erased == 1);
	ASSERT_NOTINFIND(this->userDirectory, user);

	std::cout << "Erased user " << user->Name() << " from session " << this->sessionName << std::endl;
	
	if (broadcast)
	{
		json data		= json::object();
		json usersLeft	= json::array({GenerateUserBroadcastEntry(user)});
		data["left"]	= usersLeft;
		json jsEvRemovedUser = JSUtils::CreateEvent(JSNOTIF_USERLEFT, data);
		
		this->Broadcast(jsEvRemovedUser.dump(), lockGuards);
	}
	
	return 
		(this->userDirectory.size() == 0) ?
			RemoveResult::RemovedNowEmpty :
			RemoveResult::Removed;
}

void Session::Shutdown(SessionLockedGuards& lockGuards)
{
	SessionLockedGuardsDrop guardDrop(&lockGuards, true);
	
	for (auto user : this->userDirectory)
		user->CloseConnection();

	this->userDirectory.clear();
	
	this->runningState = RunningState::Killing;
	while (this->runningState != RunningState::Dead)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	this->serviceThread.join();
}

std::vector<WSUserPtr> Session::GetUsersCopy(SessionLockedGuards& lockGuards) const
{
	SessionLockedGuardsDrop guardDrop(&lockGuards, true);
	
	std::vector<WSUserPtr> ret;
	for (WSUserPtr user : this->userDirectory)
		ret.push_back(user);

	return ret;
}

void Session::ThreadFn()
{
	VERBOSE_DEBUG("Starting session thread.");
	assert(this->runningState == RunningState::Alive);
	
	while (true)
	{
		if (this->runningState != RunningState::Alive)
			break;
		
		std::chrono::milliseconds msStart = 
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		{ // SERVICE LOOP
			
		} // END SERVICE LOOP
		std::chrono::milliseconds msEnd = 
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		if (this->runningState != RunningState::Alive)
			break;

		//VERBOSE_DEBUG("Mixing");
		std::vector<WSUserPtr> usersATM;
		{
			SessionLockedGuards lockGuard(this);
			usersATM = this->GetUsersCopy(lockGuard);
		}
		for (WSUserPtr user : usersATM)
			user->Mix();
		
		long long ellapsed = (msEnd - msStart).count();
		const int SERVICE_LOOPFRAMETIME = 100;
		if (ellapsed < SERVICE_LOOPFRAMETIME)
			std::this_thread::sleep_for(std::chrono::milliseconds(SERVICE_LOOPFRAMETIME - ellapsed));
	}
}

void Session::Broadcast(const std::string& msg, SessionLockedGuards& lockGuards)
{
	this->Broadcast(msg, std::set<WSUserPtr>(), lockGuards);
}

void Session::Broadcast(const std::string& msg, WSUserPtr ignore, SessionLockedGuards& lockGuards)
{
	this->Broadcast(msg, std::set<WSUserPtr>{ignore}, lockGuards);
}

void Session::Broadcast(const std::string& msg, std::set<WSUserPtr> ignores, SessionLockedGuards& lockGuards)
{
	SessionLockedGuardsDrop guardDrop(&lockGuards, true);
	
	for (WSUserPtr user : this->userDirectory)
	{
		if (!ignores.empty() && ignores.find(user) != ignores.end())
			continue;

		user->Send(msg);
	}
}

void Session::HandleAudio(AudioPacketPtr audioPacket, SessionLockedGuards& lockGuard)
{
	SessionLockedGuardsDrop guardDrop(&lockGuard, true);
	
	for (WSUserPtr user : this->userDirectory)
	{
		if (user == audioPacket->speaker)
			continue;

		user->QueueAudioPacket(audioPacket);
	}
}

void Session::_LockGuard_LockUsers()
{
	this->userDirectoryMutex.lock();
}

void Session::_LockGuard_UnlockUsers()
{
	this->userDirectoryMutex.unlock();
}