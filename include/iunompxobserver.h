#pragma once

#include "iunoplugin.h"
#include "complex.h"

class IUnoMpxObserver
{

public:

	virtual void MpxObserverProcess(channel_t channel, const float *buffer, int length) = 0;
};