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

#include "SDRunoPlugin_TemplateUi.h"

class SDRunoPlugin_Template : public IUnoPlugin
{

public:
	
	SDRunoPlugin_Template(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_Template();

	// TODO: change the plugin title here
	virtual const char* GetPluginName() const override { return "SDRuno Plugin Example"; }

	// IUnoPlugin
	virtual void HandleEvent(const UnoEvent& ev) override;

private:
	
	void WorkerFunction();
	std::thread* m_worker;
	std::mutex m_lock;
	SDRunoPlugin_TemplateUi m_form;
};