#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>

#include "SDRunoPlugin_DXClusterUi.h"

class SDRunoPlugin_DXCluster : public IUnoPlugin,
							   public IUnoAnnotator
{

public:
	
	SDRunoPlugin_DXCluster(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_DXCluster();

	virtual const char* GetPluginName() const override { return "SDRuno DX Cluster Plugin"; }

	// IUnoPlugin
	virtual void HandleEvent(const UnoEvent& ev) override;

	virtual void AnnotatorProcess(std::vector<IUnoAnnotatorItem>& items) override;

	void StartDXCluster(std::string addr, std::string port, std::string callsign, int timeMins);
	void StopDXCluster();
	bool IsRunning() { return m_started; }
	void StartAnnotator();
	void StopAnnotator();

	void UpdateSampleRate();

	int DXCount() { return dxCount; }

	struct DXEntry {
		std::string callsign;
		long long freq;
		std::string timeUTC;
		IUnoAnnotatorStyle style;
		DXEntry *next;
	};

	DXEntry *head, *tail, *current, *prev, *annocurr, *freqcurr;

private:
	
	void WorkerFunction();
	std::thread* m_worker;
	std::mutex m_lock;
	SDRunoPlugin_DXClusterUi m_form;
	bool m_started;
	void sendstring(int sockfd, const char *s);
	void waitforstring(int sockfd, const char *s);
	void processDX(const char *s);
	void add_DXEntry(std::string callsign, std::string t, long long freq);
	bool olderThanTimer(std::string timeUTC);
	int checkLocalFreq(long long freq, std::string callsign);

	std::string cAddr;
	std::string cPort;
	std::string cCallsign;
	int timeMinutes;
	int dxCount;
	int sampleRate;
};