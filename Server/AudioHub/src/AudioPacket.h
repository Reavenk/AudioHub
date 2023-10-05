#pragma once
#include <vector>
#include "Types.h"

class AudioPacket
{
public:
	WSUserPtr speaker = nullptr;
	std::vector<char> pcmBytes;

	AudioPacket(WSUserPtr speaker, int resizedSampleCt);

	size_t ShortSamples() const
	{ return this->pcmBytes.size() / sizeof(short); }

	short* ShortPtr()
	{ return (short*)&this->pcmBytes[0]; }
};