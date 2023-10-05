#pragma once
#include <queue>
#include <mutex>
#include "Types.h"

/// <summary>
/// A collection of consecutive streaming audio packets.
/// </summary>
class SpeakerStream
{
	enum class ClaimReturn
	{
		// Unknown error, will attempt to handle error gracefully.
		Error,
		// The packets were accounted for.
		Claimed,
		// Finished processing the elements in a packet
		Finished
	};
private:
	/// <summary>
	/// The packets queued to stream
	/// </summary>
	std::queue<AudioPacketPtr> packets;

	/// <summary>
	/// Mutual exclusion guard for packets.
	/// </summary>
	mutable std::mutex packetsMutex;

	/// <summary>
	/// For the given streaming packet, what was the last sample that was written?
	/// </summary>
	int streamCookie = 0;
public:
	/// <summary>
	/// The mixing volume for this stream
	/// </summary>
	float volume = 0.75f;

	/// <summary>
	/// Sets of the stream is enabled. If false, the stream is muted.
	/// </summary>
	bool gate = true;

public:
	AudioPacketPtr PopPacket_LOCK(int* cookie);
	AudioPacketPtr PeekPacket_LOCK(int* cookie);
	int PacketsCt_LOCK() const;
	bool PacketsEmpty_LOCK() const;
	void QueuePacket_LOCK(AudioPacketPtr packet);
	ClaimReturn ClaimStreamedSamples_LOCK(int samples);
	int TopPacketClaimable_LOCK() const;

	void Reset_LOCK(AudioPacketPtr restarting);
};