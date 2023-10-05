#pragma once

#include "Types.h"

#include <thread>
#include <set>
#include <mutex>
#include <vector>
#include "vendored/nlohmann/json.hpp"
using json = nlohmann::json;

json GenerateUserBroadcastEntry(WSUserPtr user);

class SessionLockedGuards
{
private:
	std::mutex stateMutex;
	int usersLocks = 0;
	Session* session;
public:
	void Lock(bool users);
	void Unlock(bool users);
	
	SessionLockedGuards(SessionPtr session);
	SessionLockedGuards(Session* session);
	~SessionLockedGuards();
	SessionLockedGuards(const SessionLockedGuards&) = delete;
	SessionLockedGuards& operator=(const SessionLockedGuards&) = delete;
};

class SessionLockedGuardsDrop
{
private:
	SessionLockedGuards* whatIsLocked;
	bool users;

public:
	SessionLockedGuardsDrop(SessionLockedGuards* whatIsLocked, bool users);
	~SessionLockedGuardsDrop();
	
	SessionLockedGuardsDrop(const SessionLockedGuardsDrop&) = delete;
	SessionLockedGuardsDrop& operator=(const SessionLockedGuardsDrop&) = delete;
};

class Session
{
public:
	enum class RemoveResult
	{
		Removed,
		RemovedNowEmpty,
		NotFound
	};

	enum class RunningState
	{
		Dead,
		Alive,
		Killing
	};
private:
	std::string sessionName;
	
	mutable std::mutex userDirectoryMutex;
	std::set<WSUserPtr> userDirectory;
	std::thread serviceThread;
	RunningState runningState = RunningState::Alive;

public:
	Session(const std::string& sessionName);
	~Session();
	Session& operator=(const Session& s) = delete;
	Session(const Session& s) = delete;

	void AddUser(WSUserPtr user, const std::string& action, SessionLockedGuards& lockGuard);
	bool HasUser(WSUserPtr user, SessionLockedGuards& lockGuard);

	bool Empty(SessionLockedGuards& lockGuard) const;

	RemoveResult RemoveUser(WSUserPtr user, bool broadcast, SessionLockedGuards& lockGuard);

	std::vector<WSUserPtr> GetUsersCopy(SessionLockedGuards& lockGuard) const;

	void Broadcast(const std::string& msg, SessionLockedGuards& lockGuard);
	void Broadcast(const std::string& msg, WSUserPtr ignore, SessionLockedGuards& lockGuard);
	void Broadcast(const std::string& msg, std::set<WSUserPtr> ignores, SessionLockedGuards& lockGuard);

	void HandleAudio(AudioPacketPtr audioPacket, SessionLockedGuards& lockGuard);

	void Shutdown(SessionLockedGuards& lockGuard);

	inline std::string SessionName() const
	{ return this->sessionName; }

	void ThreadFn();

public:
	void _LockGuard_LockUsers();
	void _LockGuard_UnlockUsers();
};