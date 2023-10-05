#pragma once

#include <thread>
#include "Types.h"
#include "SimpleWSinclude.h"
#include <map>
#include <set>


/// <summary>
/// For a given call stack hierarchy, manage if server mutex locks 
/// are active or not, while allowing functions to arbitrarily lock 
/// and unlock without needing to know the locking needs of parent 
/// call frames or child function calls.
/// 
/// This class should be instanciated at the top level of a thread
/// or entry point - such as websocket callbacks.
/// </summary>
class ServerLockedGuards
{
private:
	/// <summary>
	/// And lock or unlock needs to happen in a critical section
	/// </summary>
	std::mutex stateMutex;

	/// <summary>
	/// How many active user locks are there?
	/// </summary>
	int usersLocks = 0;

	/// <summary>
	/// How many active session locks are there?
	/// </summary>
	int sessionsLocks = 0;

	/// <summary>
	/// The server whos locking state is being tracked by this object.
	/// </summary>
	Server* server;
	
public:
	/// <summary>
	/// Request one or more types of locks.
	/// </summary>
	/// <param name="server">The server to lock for.</param>
	/// <param name="users">Request a mutex lock the user listings.</param>
	/// <param name="sessions">Request a mutex lock the session</param>
	void Lock(bool users, bool sessions);

	/// <summary>
	/// Request previous locks to be released.
	/// 
	/// This should have the same parameters a previous Lock() call was made with.
	/// </summary>
	void Unlock(bool users, bool sessions);

	ServerLockedGuards(Server* server);
	~ServerLockedGuards();
	ServerLockedGuards(const ServerLockedGuards&) = delete;
	ServerLockedGuards& operator=(const ServerLockedGuards&) = delete;
};

/// <summary>
/// A class that can be created on the function stack to lock resources
/// on the server, and automatically unlock them (via ServerLockedGuards)
/// when the variable's scope ends.
/// </summary>
class ServerLockedGuardsDrop
{
	/// <summary>
	/// The ServerLockedGuards that the mutex lock requests are recorded in.
	/// </summary>
	ServerLockedGuards* whatIsLocked;
	
	/// <summary>
	/// Was a user lock requested?
	/// </summary>
	bool users;

	/// <summary>
	/// Was a session lock requested?
	/// </summary>
	bool sessions;

public:
	ServerLockedGuardsDrop(ServerLockedGuards* guards, bool users, bool sessions);
	~ServerLockedGuardsDrop();

	ServerLockedGuardsDrop(const ServerLockedGuardsDrop&) = delete;
	ServerLockedGuardsDrop& operator=(const ServerLockedGuardsDrop&) = delete;
};

/// <summary>
/// Running state of the server.
/// </summary>
enum class ConState
{
	/// <summary>
	/// Server is not/no-longer running.
	/// </summary>
	Closed,
	/// <summary>
	/// Server is booting up.
	/// </summary>
	Initializing,
	/// <summary>
	/// Server is running.
	/// </summary>
	Open,
	/// <summary>
	/// Server is shutting down. Poll the connection state until Closed to detect
	/// when shutdown is complete.
	/// </summary>
	ShuttingDown
};

/// <summary>
/// AudioHub server.
/// 
/// - All text payloads are expected to be JSON messages in AudioHubs standard format.
/// - All byte payloads are expected to be PCM.
/// </summary>
class Server
{
private:
	std::thread* serverThread = nullptr;
	WsServer server;
	ConState conState = ConState::Closed;

	/// <summary>
	/// Counter for generating unique user ids.
	/// </summary>
	std::atomic<UIDTy> idCounter = 0;

	std::map<std::string, SessionPtr> sessions;
	std::mutex sessionContainersMutex;

	/// <summary>
	/// Connections that are not authorized yet.
	/// Only these connections are allowed to send login requests.
	/// </summary>
	std::set<WSConPtr> unloggedConns;

	std::map<WSConPtr, WSUserPtr> consToUsers;
	std::map<int, WSUserPtr> idToUsers;
	std::mutex userDataMutex;
	
private:
	bool RemoveLoggedInUser(WSConPtr wsCon, ServerLockedGuards& lockGuard);

public:
	Server();
	~Server();
	Server& operator=(const Server& s) = delete;
	Server(const Server& s) = delete;
	
	bool InitializeServer(int port, ServerLockedGuards& lockedGuards);
	bool ShutdownServer(ServerLockedGuards& lockedGuards);

	// Websocket handlers
	void OnWS_Opened(WSConPtr wsCon);
	void OnWS_Closed(WSConPtr wsCon, int code, const std::string& reason);
	void OnWS_Message(WSConPtr wsCon, WSInMsgPtr wsInMsg);
	void OnWS_Error(WSConPtr wsCon, const SimpleWeb::error_code& ec);

	SessionPtr GetSession(const std::string& sessionName, ServerLockedGuards& lockedGuards);

	WSUserPtr GetUser(int id, ServerLockedGuards& lockGuards);
	bool HasUser(int id, ServerLockedGuards& lockGuards);

	/// <summary>
	/// Transition a non-authorized connection to a logged in user.
	/// </summary>
	/// <param name="conToUpgrade">The connection</param>
	/// <param name="username">The username</param>
	/// <param name="sessionName">The starting session room name</param>
	/// <param name="action"></param>
	/// <param name="lockedGuards">Thread guard.</param>
	/// <returns>The logged in user.</returns>
	WSUserPtr ConvertToUser(WSConPtr conToUpgrade, const std::string& username, const std::string& sessionName, const std::string& action, ServerLockedGuards& lockedGuards);
	bool LogoutUser(WSUserPtr user, ServerLockedGuards& lockedGuards);

	bool ExitUserFromSession(WSUserPtr user, ServerLockedGuards& lockedGuards);
	
	void _ShutdownSessions(ServerLockedGuards& lockedGuards);

	inline ConState ConnectionState() const
	{ return this->conState; }

public:
	void _LockGuard_LockUsers();
	void _LockGuard_UnlockUsers();
	void _LockGuard_LockSessions();
	void _LockGuard_UnlockSessions();
	
public:
	UIDTy NewID();
};