#include "SpeakerStream.h"
#include "AudioPacket.h"
#include "Utils.h"

AudioPacketPtr SpeakerStream::PopPacket_LOCK(int* cookie)
{
	LOCKGUARD(this->packetsMutex, pack);

	if (this->packets.empty())
		return nullptr;

	if (cookie != nullptr)
		*cookie = this->streamCookie;

	this->streamCookie = 0;
	AudioPacketPtr ret = this->packets.front();
	this->packets.pop();
	return ret;
}

AudioPacketPtr SpeakerStream::PeekPacket_LOCK(int* cookie)
{
	LOCKGUARD(this->packetsMutex, pack);
	if (this->packets.empty())
		return nullptr;

	if (cookie != nullptr)
		*cookie = this->streamCookie;

	return this->packets.front();
}

int SpeakerStream::PacketsCt_LOCK() const
{
	LOCKGUARD(this->packetsMutex, pack);
	return (int)this->packets.size();
}

bool SpeakerStream::PacketsEmpty_LOCK() const
{
	LOCKGUARD(this->packetsMutex, pack);
	return this->packets.empty();
}

void SpeakerStream::QueuePacket_LOCK(AudioPacketPtr packet)
{
	LOCKGUARD(this->packetsMutex, pack);
	if (this->packets.empty())
		this->streamCookie = 0;

	this->packets.push(packet);
}

SpeakerStream::ClaimReturn SpeakerStream::ClaimStreamedSamples_LOCK(int samples)
{
	LOCKGUARD(this->packetsMutex, pack);
	if (this->packets.empty())
		return ClaimReturn::Error;

	AudioPacketPtr top = this->packets.front();
	this->streamCookie += samples;
	if (this->streamCookie > top->ShortSamples())
	{
		this->packets.pop();
		this->streamCookie = 0;
		return ClaimReturn::Error;
	}

	if (this->streamCookie == top->ShortSamples())
	{
		this->packets.pop();
		this->streamCookie = 0;
		return ClaimReturn::Finished;
	}

	return ClaimReturn::Claimed;
}

int SpeakerStream::TopPacketClaimable_LOCK() const
{
	LOCKGUARD(this->packetsMutex, pack);
	if (this->packets.empty())
		return 0;

	AudioPacketPtr top = this->packets.front();
	return (int)top->ShortSamples() - this->streamCookie;
}

void SpeakerStream::Reset_LOCK(AudioPacketPtr restarting)
{
	LOCKGUARD(this->packetsMutex, pack);

	while(!this->packets.empty())
		this->packets.pop();

	this->streamCookie = 0;

	if(restarting != nullptr)
		this->packets.push(restarting);
}