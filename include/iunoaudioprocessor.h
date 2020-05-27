#pragma once

#include "iunoplugin.h"

class IUnoAudioProcessor
{

public:

	virtual void AudioProcessorProcess(channel_t channel, float *buffer, int length, bool& modified) = 0;
};
