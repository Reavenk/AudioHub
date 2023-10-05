#pragma once
#include <queue>
#include <mutex>
#include "Types.h"

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
	// The packets queued to stream
	std::queue<AudioPacketPtr> packets;
	mutable std::mutex packetsMutex;

	// For the given streaming packet, what was the last sample that was written?
	int streamCookie = 0;
public:
	// The mixing volume for this stream
	float volume = 0.75f;
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