#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_DXCluster.h"
#include "SDRunoPlugin_DXClusterUi.h"
#include "SDRunoPlugin_DXClusterForm.h"

// Ui constructor - load the Ui control into a thread
SDRunoPlugin_DXClusterUi::SDRunoPlugin_DXClusterUi(SDRunoPlugin_DXCluster& parent, IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_started(false),
	m_controller(controller)
{
	m_thread = std::thread(&SDRunoPlugin_DXClusterUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
SDRunoPlugin_DXClusterUi::~SDRunoPlugin_DXClusterUi()
{
	StopDXCluster();
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void SDRunoPlugin_DXClusterUi::ShowUi()
{	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_DXClusterForm>(*this, m_controller);
	m_lock.unlock();

	m_form->Run();
}

bool SDRunoPlugin_DXClusterUi::IsRunning()
{
	return m_parent.IsRunning();
}

void SDRunoPlugin_DXClusterUi::StartButtonClicked(std::string addr, std::string port, std::string callsign, int timeMins, std::string response)
{
	if (!m_started)
	{
		StartDXCluster(addr, port, callsign, timeMins, response);
	}
	else
	{
		StopDXCluster();
	}
}

void SDRunoPlugin_DXClusterUi::SaveButtonClicked()
{
	SaveSettings();
}

// Load X from the ini file (if exists)
int SDRunoPlugin_DXClusterUi::LoadX()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.X", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
int SDRunoPlugin_DXClusterUi::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Y", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

std::string SDRunoPlugin_DXClusterUi::LoadCallsign()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Callsign", tmp);
	if (tmp.empty())
	{
		return "";
	}
	return tmp;
}

int SDRunoPlugin_DXClusterUi::LoadTimer()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Timer", tmp);
	if (tmp.empty())
	{
		return 5; // in minutes
	}
	return stoi(tmp);
}

int SDRunoPlugin_DXClusterUi::LoadColour()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Colour", tmp);
	if (tmp.empty())
	{
		return 1; // yellow
	}
	return stoi(tmp);
}

int SDRunoPlugin_DXClusterUi::LoadBaseline()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Baseline", tmp);
	if (tmp.empty())
	{
		return -70;
	}
	return stoi(tmp);
}

std::string SDRunoPlugin_DXClusterUi::LoadCluster()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Cluster", tmp);
	if (tmp.empty())
	{
		return "gb7baa.com:7300"; // TODO: Better default?
	}
	return tmp;
}

std::string SDRunoPlugin_DXClusterUi::LoadResponse()
{
	std::string tmp;
	m_controller.GetConfigurationKey("DXCluster.Response", tmp);
	if (tmp.empty())
	{
		return "dxspider"; // TODO: Better default?
	}
	return tmp;
}

void SDRunoPlugin_DXClusterUi::StartDXCluster(std::string addr, std::string port, std::string callsign, int timeMins, std::string response)
{
	std::lock_guard<std::mutex> l(m_lock);
	if (!m_started)
	{
		m_parent.StartDXCluster(addr, port, callsign, timeMins, response);

		if (m_form != nullptr)
		{
			m_form->SetStartButtonCaption("Stop");
			m_form->SetCallsignTextboxState(false);
			m_form->SetTimerTextboxState(false);
			m_form->SetClusterTextboxState(false);
			m_form->SetColourComboBoxState(false);
			m_form->SetBaselineTextboxState(false);
			m_form->SetResponseTextboxState(false);
			m_form->StartTimer();
		}
		m_started = true;
	}
}

void SDRunoPlugin_DXClusterUi::StopDXCluster()
{
	std::lock_guard<std::mutex> l(m_lock);
	if (m_started)
	{
		m_parent.StopDXCluster();
		if (m_form != nullptr)
		{
			m_form->StopTimer();
			m_form->SetStartButtonCaption("Start");
			m_form->SetCallsignTextboxState(true);
			m_form->SetTimerTextboxState(true);
			m_form->SetClusterTextboxState(true);
			m_form->SetColourComboBoxState(true);
			m_form->SetBaselineTextboxState(true);
			m_form->SetResponseTextboxState(true);
		}
		m_started = false;
	}
}

void SDRunoPlugin_DXClusterUi::SaveLocation()
{
	std::lock_guard<std::mutex> l(m_lock);
	nana::point position = m_form->pos();
	m_controller.SetConfigurationKey("DXCluster.X", std::to_string(position.x));
	m_controller.SetConfigurationKey("DXCluster.Y", std::to_string(position.y));
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void SDRunoPlugin_DXClusterUi::HandleEvent(const UnoEvent& ev)
{
	switch (ev.GetType())
	{
	case UnoEvent::StreamingStarted:
		break;

	case UnoEvent::StreamingStopped:
		break;

	case UnoEvent::SampleRateChanged:
		m_parent.UpdateSampleRate();
		break;

	case UnoEvent::SavingWorkspace:
		SaveLocation();
		break;

	case UnoEvent::ClosingDown:
		FormClosed();
		break;

	default:
		break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void SDRunoPlugin_DXClusterUi::FormClosed()
{
	StopDXCluster();
	m_controller.RequestUnload(&m_parent);
}

int SDRunoPlugin_DXClusterUi::DXCount()
{
	return m_parent.DXCount();
}

int SDRunoPlugin_DXClusterUi::GetColourIndex()
{
	return m_form->GetColourIndex();
}

int SDRunoPlugin_DXClusterUi::GetBaseline()
{
	return m_form->GetBaseline();
}

void SDRunoPlugin_DXClusterUi::SaveSettings()
{
	if (m_form != nullptr)
	{
		if (!m_form->GetCallsignText().empty())
		{
			m_controller.SetConfigurationKey("DXCluster.Callsign", m_form->GetCallsignText());
		}
		if (!m_form->GetTimerText().empty())
		{
			m_controller.SetConfigurationKey("DXCluster.Timer", m_form->GetTimerText());
		}
		if (!m_form->GetClusterText().empty())
		{
			m_controller.SetConfigurationKey("DXCluster.Cluster", m_form->GetClusterText());
		}
		if (!m_form->GetResponseText().empty())
		{
			m_controller.SetConfigurationKey("DXCluster.Response", m_form->GetResponseText());
		}
		m_controller.SetConfigurationKey("DXCluster.Colour", std::to_string(m_form->GetColourIndex()));
		m_controller.SetConfigurationKey("DXCluster.Baseline", std::to_string(m_form->GetBaseline()));
	}
}