#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_DXClusterForm.h"

// Forward reference
class SDRunoPlugin_DXCluster;

class SDRunoPlugin_DXClusterUi
{
public:

	SDRunoPlugin_DXClusterUi(SDRunoPlugin_DXCluster& parent, IUnoPluginController& controller);
	~SDRunoPlugin_DXClusterUi();

	void HandleEvent(const UnoEvent& evt);
	void FormClosed();

	void ShowUi();
	void StartButtonClicked(std::string addr, std::string port, std::string callsign, int timeMins);
	void SaveButtonClicked();
	bool IsRunning();

	//void StartAnnotator();
	//void StopAnnotator();

	int DXCount();

	int LoadX();
	int LoadY();
	std::string LoadCallsign();
	int LoadTimer();
	std::string LoadCluster();
	int LoadColour();
	int LoadBaseline();

	int GetColourIndex();
	int GetBaseline();

private:

	void StartDXCluster(std::string addr, std::string port, std::string callsign, int timeMins);
	void StopDXCluster();
	void SaveLocation();
	void SaveSettings();
	
	SDRunoPlugin_DXCluster & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_DXClusterForm> m_form;

	bool m_started;

	std::mutex m_lock;

	IUnoPluginController & m_controller;
};
