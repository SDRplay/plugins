#pragma once

#include "iunoplugin.h"
#include "iunostreamobserver.h"
#include "iunoaudioobserver.h"
#include "iunoaudioprocessor.h"
#include "iunostreamprocessor.h"
#include "iunompxobserver.h"
#include "iunoannotator.h"

class IUnoPluginController
{

public:

	typedef enum
	{
		DemodulatorNone = 0,
		DemodulatorAM = 1,
		DemodulatorSAM = 2,
		DemodulatorNFM = 3,
		DemodulatorMFM = 4,
		DemodulatorWFM = 5,
		DemodulatorSWFM = 6,
		DemodulatorDSB = 7,
		DemodulatorLSB = 8,
		DemodulatorUSB = 9,
		DemodulatorCW = 10,
		DemodulatorDigital = 11,
		DemodulatorDAB = 12,
		DemodulatorIQOUT = 13,
		DemodulatorADSB2 = 14,
		DemodulatorADSB8 = 15
	} DemodulatorType;

	typedef enum
	{
		DeemphasisNone = 0,
		Deemphasis50us = 1,
		Deemphasis75us = 2,
	} WfmDeemphasisMode;

	typedef enum
	{
		NoiseBlankerOff = 0,
		NoiseBlankerBwWide = 1,
		NoiseBlankerBwNarrow = 2
	} NoiseBlankerMode;

	typedef enum
	{
		AGCModeOff = 0,
		AGCModeSlow = 1,
		AGCModeMedium = 2,
		AGCModeFast = 3
	} AgcMode; 

	virtual void RegisterStreamObserver(channel_t channel,  IUnoStreamObserver* observer) = 0;
	virtual void UnregisterStreamObserver(channel_t channel, IUnoStreamObserver* observer) = 0;

	virtual void RegisterStreamProcessor(channel_t channel, IUnoStreamProcessor* observer) = 0;
	virtual void UnregisterStreamProcessor(channel_t channel, IUnoStreamProcessor* observer) = 0;

	virtual void RegisterAudioObserver(channel_t, IUnoAudioObserver* observer) = 0;
	virtual void UnregisterAudioObserver(channel_t, IUnoAudioObserver* observer) = 0;

	virtual void RegisterAudioProcessor(channel_t, IUnoAudioProcessor* observer) = 0;
	virtual void UnregisterAudioProcessor(channel_t, IUnoAudioProcessor* observer) = 0;

	virtual void RegisterMpxObserver(channel_t, IUnoMpxObserver* observer) = 0;
	virtual void UnregisterMpxObserver(channel_t, IUnoMpxObserver* observer) = 0;

	virtual void RegisterAnnotator(IUnoAnnotator *annotator) = 0;
	virtual void UnregisterAnnotator(IUnoAnnotator *annotator) = 0;

	virtual DemodulatorType GetDemodulatorType(channel_t channel) = 0;
	virtual bool SetDemodulatorType(channel_t channel, DemodulatorType type) = 0;

	virtual double GetVfoFrequency(channel_t channel) = 0;
	virtual bool SetVfoFrequency(channel_t channel, double frequency) = 0;

	virtual double GetCenterFrequency(channel_t channel) = 0;
	virtual bool SetCenterFrequency(channel_t channel, double frequency) = 0;

	virtual int GetFilterBandwidth(channel_t channel) = 0;
	virtual bool SetFilterBandwidth(channel_t channel, int bandwidth) = 0;

	virtual bool IsStreamingEnabled(channel_t channel) = 0;

	virtual double GetSampleRate(channel_t channel) = 0;
	virtual bool SetSampleRate(channel_t channel, double sampleRate) = 0;
	virtual double GetAudioSampleRate(channel_t channel) = 0;
	
	virtual bool SetIFGRRelative(channel_t channel, int ifgrIncrement) = 0;

	// Squelch
	virtual bool SetSquelchLevel(channel_t channel, int level) = 0;
	virtual int GetSquelchLevel(channel_t channel) = 0;
	virtual bool SetSquelchEnable(channel_t channel, bool enable) = 0;
	virtual bool GetSquelchEnable(channel_t channel) = 0;

	// AGC
	virtual bool SetAgcMode(channel_t channel, AgcMode mode) = 0;
	virtual AgcMode GetAgcMode(channel_t channel) = 0;
	
	virtual bool SetAgcThreshold(channel_t channel, int threshold) = 0;
	virtual int GetAgcThreshold(channel_t channel) = 0;
	
	// Noise blanker
	virtual bool SetNoiseBlankerLevel(channel_t channel, int level) = 0;
	virtual int GetNoiseBlankerLevel(channel_t channel) = 0;
	
	// Noise reduction
	virtual bool SetNoiseReductionLevel(channel_t channel, int level) = 0;
	virtual int GetNoiseReductionLevel(channel_t channel) = 0;
	
	// CW peak filter
	virtual bool SetCwPeakFilterThreshold(channel_t channel, int threshold) = 0;
	virtual int GetCwPeakFilterThreshold(channel_t channel) = 0;
	
	// FM noise reduction
	virtual bool SetFmNoiseReductionEnable(channel_t channel, bool enable) = 0;
	virtual bool GetFmNoiseReductionEnable(channel_t channel) = 0;
	
	virtual bool SetFmNoiseReductionThreshold(channel_t channel, int threshold) = 0;
	virtual int GetFmNoiseReductionThreshold(channel_t channel) = 0;

	// FM deemphasis
	virtual bool SetWfmDeemphasisMode(channel_t channel, WfmDeemphasisMode mode) = 0;
	virtual WfmDeemphasisMode GetWfmDeemphasisMode(channel_t channel) = 0;
	
	// Audio volume/muting
	virtual bool SetAudioVolume(channel_t channel, int volume) = 0;
	virtual int GetAudioVolume(channel_t channel) = 0;
	virtual bool SetAudioMute(channel_t channel, bool enable) = 0;
	virtual bool GetAudioMute(channel_t channel) = 0;

	virtual double GetSNR(channel_t channel) = 0;
	virtual double GetPower(channel_t channel) = 0;

	virtual void RequestUnload(IUnoPlugin* plugin) = 0;

	virtual bool GetConfigurationKey(std::string key, std::string& value) = 0;
	virtual bool SetConfigurationKey(std::string key, std::string value) = 0;

	virtual int GetVRXCount() = 0;
	virtual bool GetVRXEnable(channel_t channel) = 0;
	virtual bool SetVRXEnable(channel_t channel, bool enable) = 0;

	virtual int GetStepSize(channel_t channel) = 0;

	virtual int GetVFOSelect(channel_t channel) = 0;
	virtual bool SetVFOSelect(channel_t channel, int vfo) = 0;

	virtual double GetSP1MinFrequency(channel_t channel) = 0;
	virtual double GetSP1MaxFrequency(channel_t channel) = 0;

	virtual double GetMPXLevel(channel_t channel) = 0;
	virtual bool SetMPXLevel(channel_t channel, double mpxLevel) = 0;

	virtual bool GetBiasTEnable() = 0;
	virtual bool SetBiasTEnable(bool enable) = 0;

	virtual int GetSP1MinPower(channel_t channel) = 0;
	virtual int GetSP1MaxPower(channel_t channel) = 0;
};