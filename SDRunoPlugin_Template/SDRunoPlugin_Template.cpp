#include <sstream>
#include <unoevent.h>
#include <iunoplugincontroller.h>
#include <vector>
#include <sstream>
#include <chrono>

#include "SDRunoPlugin_Template.h"
#include "SDRunoPlugin_TemplateUi.h"

SDRunoPlugin_Template::SDRunoPlugin_Template(IUnoPluginController& controller) :
	IUnoPlugin(controller),
	m_form(*this, controller),
	m_worker(nullptr)
{
}

SDRunoPlugin_Template::~SDRunoPlugin_Template()
{	
}

void SDRunoPlugin_Template::HandleEvent(const UnoEvent& ev)
{
	m_form.HandleEvent(ev);	
}

void SDRunoPlugin_Template::WorkerFunction()
{
	// Worker Function Code Goes Here
}