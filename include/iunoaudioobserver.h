#pragma once

#include "iunoplugin.h"

class IUnoAudioObserver
{

public:

	virtual void AudioObserverProcess(channel_t channel, const float *buffer, int length) = 0;
};
