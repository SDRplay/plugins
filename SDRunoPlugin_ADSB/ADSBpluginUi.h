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
#include "ADSBpluginForm.h"

// Forward reference
class ADSBplugin;

class ADSBpluginUi
{
public:

	ADSBpluginUi(ADSBplugin& parent, IUnoPluginController& controller);
	~ADSBpluginUi();

	void HandleEvent(const UnoEvent& ev);
	void FormClosed();

	void ShowUi();
	void StartButtonClicked();
	void SetOversample(bool checked);
	bool GetOverSample();
	void GainControl(bool checked);

	void ClearDisplay();
	void PutDisplayString(char* str);

	std::string http_ports;
	std::string raw_ports;
	std::string sbs_ports;
	std::string beast_ports;

private:

	void EnableAdsb();
	void DisableAdsb();
	void StartAdsb();
	void StopAdsb();

	ADSBplugin& m_parent;
	std::thread m_thread;
	std::shared_ptr<ADSBpluginForm> m_form;

	bool m_started;
	bool m_oversample;

	std::mutex m_lock;

	IUnoPluginController& m_controller;
};
