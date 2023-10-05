#pragma once
#include <vector>
#include "Types.h"

class AudioPacket
{
public:
	/// <summary>
	/// The user who provided the AudioPacket.
	/// </summary>
	WSUserPtr speaker = nullptr;

	/// <summary>
	/// The PCM data of the audio sample.
	/// </summary>
	std::vector<char> pcmBytes;

	AudioPacket(WSUserPtr speaker, int resizedSampleCt);

	size_t ShortSamples() const
	{ return this->pcmBytes.size() / sizeof(short); }

	short* ShortPtr()
	{ return (short*)&this->pcmBytes[0]; }
};