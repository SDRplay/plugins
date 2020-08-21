#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_Template.h"
#include "SDRunoPlugin_TemplateUi.h"
#include "SDRunoPlugin_TemplateForm.h"

// Ui constructor - load the Ui control into a thread
SDRunoPlugin_TemplateUi::SDRunoPlugin_TemplateUi(SDRunoPlugin_Template& parent, IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_controller(controller)
{
	m_thread = std::thread(&SDRunoPlugin_TemplateUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
SDRunoPlugin_TemplateUi::~SDRunoPlugin_TemplateUi()
{	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void SDRunoPlugin_TemplateUi::ShowUi()
{	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_TemplateForm>(*this, m_controller);
	m_lock.unlock();

	m_form->Run();
}

// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TemplateUi::LoadX()
{
	std::string tmp;
	m_controller.GetConfigurationKey("Template.X", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TemplateUi::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("Template.Y", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void SDRunoPlugin_TemplateUi::HandleEvent(const UnoEvent& ev)
{
	switch (ev.GetType())
	{
	case UnoEvent::StreamingStarted:
		break;

	case UnoEvent::StreamingStopped:
		break;

	case UnoEvent::SavingWorkspace:
		break;

	case UnoEvent::ClosingDown:
		FormClosed();
		break;

	default:
		break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void SDRunoPlugin_TemplateUi::FormClosed()
{
	m_controller.RequestUnload(&m_parent);
}
