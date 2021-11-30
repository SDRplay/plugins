#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "ADSBprint.h"
#include "ADSBplugin.h"
#include "ADSBpluginUi.h"
#include "ADSBpluginForm.h"
#include "ADSBpluginSettingsDialog.h"

ADSBpluginUi::ADSBpluginUi(ADSBplugin& parent, IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_started(false),
	m_oversample(false),
	m_controller(controller)
{
	m_thread = std::thread(&ADSBpluginUi::ShowUi, this);
}

ADSBpluginUi::~ADSBpluginUi()
{
	DEBUG_PRINT("ADSBpluginUi::~ADSBpluginUi\n");

	StopAdsb();
	m_controller.SetDemodulatorType(0, IUnoPluginController::DemodulatorNone);
	nana::API::exit_all();
	m_thread.join();

	DEBUG_PRINT("ADSBpluginUi::~ADSBpluginUi end\n");
}

void ADSBpluginUi::ShowUi()
{
	DEBUG_PRINT("ADSBpluginUi::ShowUi\n");

	m_lock.lock();
	m_form = std::make_shared<ADSBpluginForm>(*this, m_controller);
	m_lock.unlock();

	m_form->Run();
}

void ADSBpluginUi::ClearDisplay()
{
	m_form->ClearDisplay();
}

void ADSBpluginUi::PutDisplayString(char* str)
{
	m_form->PutDisplayString(str);
}

void ADSBpluginUi::HandleEvent(const UnoEvent& ev)
{
	switch (ev.GetType())
	{
	case UnoEvent::StreamingStarted:
		DEBUG_PRINT("ADSBpluginUi::HandleEvent - StreamingStarted\n");
		EnableAdsb();
		break;

	case UnoEvent::StreamingStopped:
		DEBUG_PRINT("ADSBpluginUi::HandleEvent - StreamingStopped\n");

		if (m_started)
		{
			StopAdsb();
		}
		DisableAdsb();
		break;

	case UnoEvent::IFGainChanged:
		DEBUG_PRINT("ADSBpluginUi::HandleEvent - IFGainChanged\n");

		m_parent.GrChanged();
		if (m_form)
		{
			bool en = m_controller.IsStreamingEnabled(0);
			DEBUG_PRINT("ADSBpluginUi::HandleEvent - m_form->SetStartButtonState(%s)\n", (en == true) ? "true" : "false");
			m_form->SetStartButtonState(en);
		}
		break;

	case UnoEvent::SavingWorkspace:
		m_form->SavePosition();
		break;

	case UnoEvent::ClosingDown:
		StopAdsb();
		m_controller.SetDemodulatorType(0, IUnoPluginController::DemodulatorNone);
		FormClosed();
		break;

	case UnoEvent::DemodulatorChanged:
		if ((m_controller.GetDemodulatorType(0) == IUnoPluginController::DemodulatorADSB2) || (m_controller.GetDemodulatorType(0) == IUnoPluginController::DemodulatorADSB8))
		{
			DEBUG_PRINT("ADSBpluginUi::HandleEvent - DemodulatorChanged\n");
			if (m_form)
			{
				bool en = m_controller.IsStreamingEnabled(0);
				DEBUG_PRINT("ADSBpluginUi::HandleEvent - m_form->SetStartButtonState(%s)\n", (en == true) ? "true" : "false");
				m_form->SetStartButtonState(en);
			}
		}
		break;

	case UnoEvent::StartRequest:
		if (!m_started)
		{
			int count = 0;
			while (!m_form)
			{
				nana::system::sleep(1000);
				count++;
				if (count > 5)
				{
					break;
				}
			}

			count = 0;
			while (m_form->GetStartButtonState() == false)
			{
				nana::system::sleep(1000);
				count++;
				if (count > 5)
				{
					break;
				}
			}

			StartButtonClicked();
		}
		break;

	case UnoEvent::StopRequest:
		if (m_started)
		{
			if (m_form)
			{
				if (m_form->GetStartButtonState())
				{
					StartButtonClicked();
				}
			}
		}
		break;

	default:
		break;
	}
}

void ADSBpluginUi::FormClosed()
{
	DEBUG_PRINT("ADSBpluginUi::FormClosed\n");
	m_controller.RequestUnload(&m_parent);
}

void ADSBpluginUi::StartButtonClicked()
{
	DEBUG_PRINT("ADSBpluginUi::StartButtonClicked\n");
	if (!m_started)
	{
		SetOversample(m_oversample);
		m_form->ClearDisplay();
		StartAdsb();
	}
	else
	{
		StopAdsb();
	}
}

void ADSBpluginUi::EnableAdsb()
{
	DEBUG_PRINT("ADSBpluginUi::EnableAdsb\n");

	std::lock_guard<std::mutex> l(m_lock);
	if (m_form != nullptr)
	{
		m_form->SetStartButtonState(true);
	}
}

void ADSBpluginUi::DisableAdsb()
{
	DEBUG_PRINT("ADSBpluginUi::DisableAdsb\n");

	std::lock_guard<std::mutex> l(m_lock);
	if (m_form != nullptr)
	{
		m_form->SetStartButtonState(false);
	}
}

void ADSBpluginUi::StartAdsb()
{
	DEBUG_PRINT("ADSBpluginUi::StartAdsb\n");

	std::lock_guard<std::mutex> l(m_lock);
	if (!m_started)
	{
		m_parent.SetPorts(http_ports, raw_ports, sbs_ports, beast_ports);
		m_parent.StartAdsb();

		if (m_form != nullptr)
		{
			m_form->SetStartButtonCaption("Stop");
		}
		m_started = true;
	}
}

void ADSBpluginUi::StopAdsb()
{
	DEBUG_PRINT("ADSBpluginUi::StopAdsb\n");

	std::lock_guard<std::mutex> l(m_lock);
	if (m_started)
	{
		m_parent.StopAdsb();
		if (m_form != nullptr)
		{
			m_form->SetStartButtonCaption("Start");
		}
		m_started = false;
	}
}

void ADSBpluginUi::SetOversample(bool checked)
{
	DEBUG_PRINT("ADSBpluginUi::SetOversample(%s)\n", (checked == true) ? "true" : "false");

	m_oversample = checked;
	m_parent.SetAdsbMode(checked);
}

bool ADSBpluginUi::GetOverSample()
{
	return m_oversample;
}

void ADSBpluginUi::GainControl(bool checked)
{
	DEBUG_PRINT("ADSBpluginUi::GainControl(%s)\n", (checked == true) ? "true" : "false");

	m_parent.SetGainControl((checked == true) ? 1 : 0);
}

