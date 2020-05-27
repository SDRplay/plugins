#pragma once

#include "iunoplugin.h"
#include "complex.h"

class IUnoStreamObserver
{

public:

	virtual void StreamObserverProcess(channel_t channel, const Complex *buffer, int length) = 0;
};
