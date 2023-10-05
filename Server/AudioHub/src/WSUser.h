#pragma once
#include "Types.h"
#include "SimpleWSinclude.h"
#include <string>
#include "vendored/nlohmann/json.hpp"
#include <chrono>
#include "defines.h"

using json = nlohmann::json;

const int TenthSecondSamples = SAMPLERATE / 10;

/// <summary>
/// A user who is logged into the server
/// </summary>
class WSUser
{
private:
	/// <summary>
	/// Unique ID of the user - used to address the user in the APIs.
	/// </summary>
	UIDTy uniqueID;

	/// <summary>
	/// The username.
	/// </summary>
	std::string name;

	/// <summary>
	/// The Websocket connection.
	/// </summary>
	WSConPtr wsCon;

	/// <summary>
	/// The chatroom the user is participating in.
	/// </summary>
	SessionPtr session;

	/// <summary>
	/// The mutual exclusion lock to access speakersAudioToMix.
	/// </summary>
	std::mutex speakersStreamMutex;

	/// <summary>
	/// Output audio buffer to process and stream to the user.
	/// </summary>
	std::map<WSUserPtr, SpeakerStreamPtr> speakersAudioToMix;

	/// <summary>
	/// The last time the stream was processed specifically for this stream.
	/// </summary>
	long long lastStreamed;
private:
	SpeakerStreamPtr GetSpeakerStream(WSUserPtr user);
	
public:
	WSUser(WSConPtr wscon, const std::string& name, UIDTy uid, SessionPtr session);
	~WSUser();
	WSUser& operator=(const WSUser& u) = delete;
	WSUser(const WSUser& u) = delete;

	void _ResetSession(SessionPtr session);

	inline UIDTy ID() const
	{ return this->uniqueID; }

	inline std::string Name() const
	{ return this->name; }

	SessionPtr GetSession();

	inline WSConPtr GetWSCon() const
	{ return this->wsCon; }

	std::string SessionName() const;

	void Send(const std::string& payload);
	void Send(const json& payload);
	void SendAudio(const char* pcPCMBin, size_t bufferLen);

	void SetVolume_LOCK(WSUserPtr user, float volume);
	void SetVolume(WSUserPtr user, float volume);
	void SetGate_LOCK(WSUserPtr user, bool gate);
	void SetGate(WSUserPtr user, bool gate);

	void ClearStreams();
	void ClearStreams_LOCK();

	void RebufferStreams_LOCK(const std::vector<int>& toRebuff, int samplesToRebuff);
	
	void QueueAudioPacket(AudioPacketPtr audioPacket);
	
	std::vector<SpeakerStreamPtr> GetSpeakerStreamCopy();

	// Mix the streaming audio and send the mixed PCM to the user's WebSocket connection.
	void Mix(int minSamples = TenthSecondSamples);

	void RemoveStreamsFromUser(WSUserPtr user);
	
	void CloseConnection();

	void Destroy();
};