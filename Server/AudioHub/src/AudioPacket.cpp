#include "AudioPacket.h"

AudioPacket::AudioPacket(WSUserPtr speaker, int resizedSampleCt)
{
	this->speaker = speaker;
	this->pcmBytes.resize(resizedSampleCt * sizeof(short));
}