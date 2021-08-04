#pragma once

#include "iunoplugin.h"

class UnoEvent
{

public:

	typedef enum
	{
		UndefinedEvent = 0,
		
		DemodulatorChanged = 1,
		BandwidthChanged = 2,
		FrequencyChanged = 3,
		CenterFrequencyChanged = 4,
		SampleRateChanged = 5,
		StreamingStarted = 6,
		StreamingStopped = 7,
		SquelchEnableChanged = 8,
		SquelchThresholdChanged = 9,
		AgcThresholdChanged = 10,
		AgcModeChanged = 11,
		NoiseBlankerLevelChanged = 12,
		NoiseReductionLevelChanged = 13,
		CwPeakFilterThresholdChanged = 14,
		FmNoiseReductionEnabledChanged = 15,
		FmNoiseReductionThresholdChanged = 16,
		WfmDeemphasisModeChanged = 17,
		AudioVolumeChanged = 18,
		AudioMuteChanged = 19,
		IFGainChanged = 20,
		SavingWorkspace = 21,
		VRXCountChanged = 22,
		VRXStateChanged = 23,
		StepSizeChanged = 24,
		VFOChanged = 25,
		ClosingDown = 26,
		SP1MinFreqChanged = 27,
		SP1MaxFreqChanged = 28,
		BiasTEnableChanged = 29,
		SP1MinPowerChanged = 30,
		SP1MaxPowerChanged = 31,
		StartRequest = 32,
		StopRequest = 33,
		VFOSignalPresent = 34,
		VFOSignalNotPresent = 35
	} UnoEventType;

	UnoEvent() :
		UnoEvent(UnoEvent::UndefinedEvent, 0)
	{

	}

	UnoEvent(UnoEventType type, channel_t channel) :
		m_type(type),
		m_channel(channel)
	{

	}

	const char* ToString() const
	{
		switch (m_type)
		{
		case DemodulatorChanged:
			return "DemodulatorChanged";
		case BandwidthChanged:
			return "BandwidthChanged";
		case FrequencyChanged:
			return "FrequencyChanged";
		case CenterFrequencyChanged:
			return "CentreFrequencyChanged";
		case SampleRateChanged:
			return "SampleRateChanged";
		case StreamingStarted:
			return "StreamingStarted";
		case StreamingStopped:
			return "StreamingStopped";
		case SquelchEnableChanged:
			return "SquelchEnableChanged";
		case SquelchThresholdChanged:
			return "SquelchThresholdChanged";
		case AgcThresholdChanged:
			return "AgcThresholdChanged";
		case AgcModeChanged:
			return "AgcModeChanged";
		case NoiseBlankerLevelChanged:
			return "NoiseBlankerLevelChanged";
		case NoiseReductionLevelChanged:
			return "NoiseReductionLevelChanged";
		case CwPeakFilterThresholdChanged:
			return "CwPeakFilterThresholdChanged";
		case FmNoiseReductionEnabledChanged:
			return "fmNoiseReductionEnabledChanged";
		case FmNoiseReductionThresholdChanged:
			return "FmNoiseReductionThresholdChanged";
		case WfmDeemphasisModeChanged:
			return "WfmDeemphasisModeChanged";
		case AudioVolumeChanged:
			return "AudioVolumeChanged";
		case AudioMuteChanged:
			return "AudioMuteChanged";
		case IFGainChanged:
			return "IFGainChanged";
		case SavingWorkspace:
			return "SavingWorkspace";
		case VRXCountChanged:
			return "VRXCountChanged";
		case VRXStateChanged:
			return "VRXStateChanged";
		case StepSizeChanged:
			return "StepSizeChanged";
		case VFOChanged:
			return "VFOChanged";
		case ClosingDown:
			return "ClosingDown";
		case SP1MinFreqChanged:
			return "SP1MinFreqChanged";
		case SP1MaxFreqChanged:
			return "SP1MaxFreqChanged";
		case BiasTEnableChanged:
			return "BiasTEnableChanged";
		case SP1MinPowerChanged:
			return "SP1MinPowerChanged";
		case SP1MaxPowerChanged:
			return "SP1MaxPowerChanged";
		case StartRequest:
			return "StartRequest";
		case StopRequest:
			return "StopRequest";
		case VFOSignalPresent:
			return "VFOSignalPresent";
		case VFOSignalNotPresent:
			return "VFOSignalNotPresent";
		default:
			return "Undefined event";
		}
	}

	UnoEventType GetType() const { return m_type; }
	channel_t GetChannel() const { return m_channel; }

private:

	UnoEventType m_type;
	channel_t m_channel;
};
