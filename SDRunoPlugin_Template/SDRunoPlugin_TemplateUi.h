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
#include "SDRunoPlugin_TemplateForm.h"

// Forward reference
class SDRunoPlugin_Template;

class SDRunoPlugin_TemplateUi
{
public:

	SDRunoPlugin_TemplateUi(SDRunoPlugin_Template& parent, IUnoPluginController& controller);
	~SDRunoPlugin_TemplateUi();

	void HandleEvent(const UnoEvent& evt);
	void FormClosed();

	void ShowUi();

	int LoadX();
	int LoadY();

private:
	
	SDRunoPlugin_Template & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_TemplateForm> m_form;

	bool m_started;

	std::mutex m_lock;

	IUnoPluginController & m_controller;
};
