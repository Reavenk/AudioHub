#pragma once

#include "Types.h"

#include <thread>
#include <set>
#include <mutex>
#include <vector>
#include "vendored/nlohmann/json.hpp"
using json = nlohmann::json;

json GenerateUserBroadcastEntry(WSUserPtr user);

// A lock for a Session, where multiple things on the same 
// thread can request locking. If multiple things on the same thread 
// lock, thread access permissions do not change, but multiple locks 
// for the thread are reference counted.
class SessionLockedGuards
{
private:
	std::mutex stateMutex;
	int usersLocks = 0;
	Session* session;
public:
	// The class is designed to lock multiple types of resources, but for
	// now only user resources are supported.

	/// <summary>
	/// Lock resources in the session.
	/// </summary>
	/// <param name="users">If true, the user lock is incremented..</param>
	void Lock(bool users);
	/// <summary>
	/// Unlock resources in the session.
	/// </summary>
	/// <param name="users">If true, the user lock is decremented.</param>
	void Unlock(bool users);
	
	SessionLockedGuards(SessionPtr session);
	SessionLockedGuards(Session* session);
	~SessionLockedGuards();
	SessionLockedGuards(const SessionLockedGuards&) = delete;
	SessionLockedGuards& operator=(const SessionLockedGuards&) = delete;
};

/// <summary>
/// A self-scoping automation utility to lock a SessionLockedGuards.
/// 
/// Creating one of these classes on the stack allows locking and unlocking
/// a SessionLockedGuards for the duration of code scope.
/// </summary>
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

/// <summary>
/// A grouping of logged in users that can broadcast audio and JSON payloads to
/// each other. Similar to a chatroom or a (single) subscribed topic.
/// </summary>
class Session
{
public:
	/// <summary>
	/// Details return results for RemoveUser().
	/// </summary>
	enum class RemoveResult
	{
		/// <summary>
		/// User was successfully removed, and there are still other users in
		/// the session.
		/// </summary>
		Removed,
		/// <summary>
		/// User was succesfully removed, and was the last user in the session.
		/// The session is now empty.
		/// </summary>
		RemovedNowEmpty,
		/// <summary>
		/// User did not exist in the session.
		/// </summary>
		NotFound
	};

	/// <summary>
	/// The various running states of a session.
	/// </summary>
	enum class RunningState
	{
		/// <summary>
		/// The session is not/no-longer running.
		/// </summary>
		Dead,
		/// <summary>
		/// The session is running.
		/// </summary>
		Alive,
		/// <summary>
		/// The session is shutting down. Poll the running state
		/// until Dead to detect when shutdown is finished.
		/// </summary>
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

	/// <summary>
	/// Check if the Session is empty of users.
	/// </summary>
	bool Empty(SessionLockedGuards& lockGuard) const;

	RemoveResult RemoveUser(WSUserPtr user, bool broadcast, SessionLockedGuards& lockGuard);

	/// <summary>
	/// Get a copy of the users in the session.
	/// This incurs the cost of copying a vector of smart pointers, but
	/// has the advantage that afterwards, you will have a snapshot of users
	/// at a given moment, that isn't locking the thread to guard the
	/// userDirectory.
	/// </summary>
	std::vector<WSUserPtr> GetUsersCopy(SessionLockedGuards& lockGuard) const;

	void Broadcast(const std::string& msg, SessionLockedGuards& lockGuard);
	void Broadcast(const std::string& msg, WSUserPtr ignore, SessionLockedGuards& lockGuard);
	void Broadcast(const std::string& msg, std::set<WSUserPtr> ignores, SessionLockedGuards& lockGuard);

	/// <summary>
	/// Distribute a packet of input streaming audio for the users in the session.
	/// </summary>
	void HandleAudio(AudioPacketPtr audioPacket, SessionLockedGuards& lockGuard);

	void Shutdown(SessionLockedGuards& lockGuard);

	inline std::string SessionName() const
	{ return this->sessionName; }

	/// <summary>
	/// Main thread loop function.
	/// </summary>
	void ThreadFn();

public:
	void _LockGuard_LockUsers();
	void _LockGuard_UnlockUsers();
};