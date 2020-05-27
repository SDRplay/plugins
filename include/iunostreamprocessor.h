#pragma once

#include "iunoplugin.h"
#include "complex.h"

class IUnoStreamProcessor
{

public:

	virtual void StreamProcessorProcess(channel_t channel, Complex *buffer, int length, bool& modified) = 0;
};
