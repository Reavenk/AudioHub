#include "WSUser.h"
#include "Session.h"
#include "SpeakerStream.h"
#include "AudioPacket.h"
#include "Utils.h"
#include "Server.h"

long long MicrosecondTimeStampNow()
{
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

WSUser::WSUser(WSConPtr wscon, const std::string& name, UIDTy uid, SessionPtr session)
{
	this->wsCon = wscon;
	this->name = name;
	this->session = session;
	this->uniqueID = uid;
	this->lastStreamed = MicrosecondTimeStampNow();
}

WSUser::~WSUser()
{
	std::cout << "Destroying user " << this->Name() << std::endl;
	this->Destroy();
}

void WSUser::_ResetSession(SessionPtr session)
{
	assert(session != nullptr);
	assert(this->session != session);
	this->session = session;
}

void WSUser::Destroy()
{
	this->wsCon.reset();
	this->session.reset();
	{
		LOCKGUARD(this->speakersStreamMutex, speakers);
		this->speakersAudioToMix.clear();
	}
}

SessionPtr WSUser::GetSession()
{
	return this->session;
}

SpeakerStreamPtr WSUser::GetSpeakerStream(WSUserPtr user)
{
	auto itFind = this->speakersAudioToMix.find(user);
	if (itFind != this->speakersAudioToMix.end())
		return itFind->second;

	SpeakerStreamPtr speakerStream = SpeakerStreamPtr(new SpeakerStream());
	this->speakersAudioToMix[user] = speakerStream;
	return speakerStream;
}

std::string WSUser::SessionName() const
{
	assert(this->session != nullptr);
	return this->session->SessionName();
}

void WSUser::Send(const std::string& payload)
{
	if (this->wsCon == nullptr)
		return;
	
	this->wsCon->send(payload);
}

void WSUser::Send(const json& payload)
{
	// This is already done in the Send overload, it's cheap enough to not
	// worry about doing it again redundantly.
	if (this->wsCon == nullptr)
		return;

	this->Send(payload.dump());
}

void WSUser::SendAudio(const char* pcPCMBin, size_t bufferLen)
{
	if (this->wsCon == nullptr)
		return;

#define WS_BINARYCODE 130U
	
	std::shared_ptr<WsServer::OutMessage> outMsgPtr = std::make_shared<WsServer::OutMessage>();
	outMsgPtr->write(pcPCMBin, bufferLen);
	this->wsCon->send(outMsgPtr, nullptr, WS_BINARYCODE);
}

void WSUser::SetVolume_LOCK(WSUserPtr user, float volume)
{
	LOCKGUARD(this->speakersStreamMutex, speakers);
	this->SetVolume(user, volume);
}

void WSUser::SetVolume(WSUserPtr user, float volume)
{
	SpeakerStreamPtr stream = this->GetSpeakerStream(user);
	stream->volume = volume;
}

void WSUser::SetGate_LOCK(WSUserPtr user, bool gate)
{
	LOCKGUARD(this->speakersStreamMutex, speakers);
	this->SetGate(user, gate);
}

void WSUser::SetGate(WSUserPtr user, bool gate)
{
	SpeakerStreamPtr stream = this->GetSpeakerStream(user);
	stream->gate = gate;
}

void WSUser::QueueAudioPacket(AudioPacketPtr audioPacket)
{
	SpeakerStreamPtr stream = this->GetSpeakerStream(audioPacket->speaker);
	stream->QueuePacket_LOCK(audioPacket);

}

void WSUser::ClearStreams()
{
	this->speakersAudioToMix.clear();
}

void WSUser::ClearStreams_LOCK()
{
	LOCKGUARD(this->speakersStreamMutex, speakers);
	this->ClearStreams();
}

void WSUser::RebufferStreams_LOCK(const std::vector<int>& toRebuff, int samplesToRebuff)
{
	LOCKGUARD(this->speakersStreamMutex, speakers);

	samplesToRebuff = std::min(samplesToRebuff, SAMPLERATE * 4);
	if (toRebuff.empty())
	{
		for (auto it : this->speakersAudioToMix)
			it.second->Reset_LOCK(std::make_shared<AudioPacket>(it.first, samplesToRebuff));
	}
	else
	{
		std::set<int> uniqueToRebuffs(toRebuff.begin(), toRebuff.end());
		for (auto it : this->speakersAudioToMix)
		{
			if (uniqueToRebuffs.find(it.first->ID()) == uniqueToRebuffs.end())
				continue;

			it.second->Reset_LOCK(std::make_shared<AudioPacket>(it.first, samplesToRebuff));
		}
	}
}

std::vector<SpeakerStreamPtr> WSUser::GetSpeakerStreamCopy()
{
	LOCKGUARD(this->speakersStreamMutex, speakers);
	std::vector<SpeakerStreamPtr> ret;
	ret.reserve(this->speakersAudioToMix.size());
	for (auto it : this->speakersAudioToMix)
		ret.push_back(it.second);

	return ret;
}

void WSUser::Mix(int minSamples)
{
	// Figure out how many samples we're going to process
	long long timestamp = MicrosecondTimeStampNow();
	long long timeDiffMicro = timestamp - this->lastStreamed;
	long long samples = (long long)(double(timeDiffMicro * SAMPLERATE) / MILLION_D);
	// If we're not going to mix that many samples, hold off 
	// for now and we'll defer it later for a (slightly) bigger workload.
	if (samples < minSamples)
		return;

	// In an attempt to get rid of rounding error, we 
	// based the time based on how many samples we're going
	// to process, instead of the current time.
	this->lastStreamed = (long long)(this->lastStreamed + (samples * MILLION_D) / SAMPLERATE);

	// Setup the buffer to mix the audio packet in.
	//
	// We're going to make the (ATM) assumption that it's less
	// cumbersome to mix as floats, and then convert back
	// to shorts, instead of staying with shorts the entire time.
	//
	// It definitly should make it easier for clamping in the [-1, 1]
	// range, since with shorts we wouldn't have the effective precision.
	std::vector<float> mixedPCM;
	mixedPCM.resize(samples);
	for (int i = 0; i < samples; ++i)
		mixedPCM[i] = 0.0;

	// Get a copy of all the streams, so we don't bogart the
	// streams while microphone streams are continually trying
	// to add to it.
	std::vector<SpeakerStreamPtr> vec = this->GetSpeakerStreamCopy();
	for(SpeakerStreamPtr speakerData : vec)
	{
		// Each user has a per-volume control in respect to every other listener.
		const float speakerVol = speakerData->volume;
	
		int samplesIt = 0;
		while (samplesIt < samples)
		{
			int cookie;
			AudioPacketPtr packet = speakerData->PeekPacket_LOCK(&cookie);
			if (packet == nullptr)
				break;
	
			short* shortSamples = packet->ShortPtr();
	
			// We either dump the entire audio sample, or dump 
			// as much as we can before we run out of the buffer
			// space that we're supposed to fill for this iteration 
			// of Mix().
			int samplesToProcess = std::min(speakerData->TopPacketClaimable_LOCK(), (int)(samples - samplesIt));
			if (speakerData->gate)
			{
				for (int i = 0; i < samplesToProcess; ++i)
				{
					float sample = shortSamples[cookie + i] / (float)std::numeric_limits<short>::max();
					mixedPCM[samplesIt + i] = sample * speakerVol;
				}
			}
	
			samplesIt += samplesToProcess;
			speakerData->ClaimStreamedSamples_LOCK(samplesToProcess);
		}
	}
	//static float circpos = 0.0f;
	//float incr = 440.0f / (6.28f * 160.0f);
	//for (int i = 0; i < samples; ++i)
	//{
	//	mixedPCM[i] = sin(circpos) + sin(circpos / 8.0f);
	//	circpos += incr;
	//}

	std::vector<short> streamAsShort;
	streamAsShort.resize(mixedPCM.size());
	for (int i = 0; i < mixedPCM.size(); ++i)
	{
		float clamped = std::min(1.0f, std::max(-1.0f, mixedPCM[i]));
		streamAsShort[i] = (short)(clamped * std::numeric_limits<short>::max());
	}

	this->SendAudio((const char*)&streamAsShort[0], sizeof(short) * streamAsShort.size());
}

void WSUser::RemoveStreamsFromUser(WSUserPtr user)
{
	LOCKGUARD(this->speakersStreamMutex, speakers);
	auto itFind = this->speakersAudioToMix.find(user);
	if (itFind != this->speakersAudioToMix.end())
		this->speakersAudioToMix.erase(itFind);
}

void WSUser::CloseConnection()
{
	if (this->wsCon == nullptr)
		return;

	this->wsCon->send_close(0, "Server requested close connection.");
}