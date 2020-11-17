#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

#define formWidth (297)
#define formHeight (240)

class SDRunoPlugin_DXClusterUi;

class SDRunoPlugin_DXClusterForm : public nana::form
{

public:

	SDRunoPlugin_DXClusterForm(SDRunoPlugin_DXClusterUi& parent, IUnoPluginController& controller);
	~SDRunoPlugin_DXClusterForm();

	void SetStartButtonCaption(std::string text)
	{
		startBtn.caption(text);
	}

	void SetCallsignTextboxState(bool state)
	{
		callsignTb.enabled(state);
	}

	void SetTimerTextboxState(bool state)
	{
		timerTb.enabled(state);
	}

	void SetResponseTextboxState(bool state)
	{
		responseTb.enabled(state);
	}

	void SetClusterTextboxState(bool state)
	{
		clusterTb.enabled(state);
	}

	void SetColourComboBoxState(bool state)
	{
		colourCb.enabled(state);
	}

	void SetBaselineTextboxState(bool state)
	{
		baselineTb.enabled(state);
	}

	std::string GetCallsignText()
	{
		return callsignTb.text();
	}

	std::string GetTimerText()
	{
		return timerTb.text();
	}

	std::string GetClusterText()
	{
		return clusterTb.text();
	}

	std::string GetResponseText()
	{
		return responseTb.text();
	}

	int GetColourIndex()
	{
		return colourCb.option();
	}
	
	int GetBaseline()
	{
		if(baselineTb.text().empty()) return -70;
		return stoi(baselineTb.text());
	}

	std::string colours[6] = {
		"Red", "Yellow", "Purple", "Green", "Light Blue", "White"
	};
	
	void Run();

	void StartTimer()
	{
		m_timerCount = 0.0;
		m_timer.start();
	}

	void StopTimer()
	{
		m_timer.stop();
	}
	
private:

	void Setup();

	// Set these two to be relative to the size of the overall form
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	// Added topBarHeight to accomodate new header style
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	// Add an "invisible" label the same size as the form to act as drag trigger for form
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	// Add images to hold bitmaps for button states...
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	// Add pictures to act as header buttons...
	nana::picture close_button{ *this, nana::rectangle(8, 25, 20, 15) };
	nana::picture min_button{ *this, nana::rectangle(8, 25, 20, 15) };
	// Rest of UI form
	nana::label callsignLbl{ *this, nana::rectangle(18, 37, 50, 20) };
	nana::textbox callsignTb{ *this, nana::rectangle(68, 35, 80, 20) };
	nana::label clusterLbl{ *this, nana::rectangle(18, 62, 50, 20) };
	nana::textbox clusterTb{ *this, nana::rectangle(68, 60, 210, 20) };
	nana::label timerLbl{ *this, nana::rectangle(158, 37, 70, 20) };
	nana::textbox timerTb{ *this, nana::rectangle(228, 35, 50, 20) };
	nana::button startBtn{ *this, nana::rectangle(18, 165, 80, 20) };
	nana::button saveBtn{ *this, nana::rectangle(18, 195, 80, 20) };
	nana::label  baselineLbl{ *this, nana::rectangle(18, 87, 80, 20) };
	nana::textbox baselineTb{ *this, nana::rectangle(98, 85, 40, 20) };
	nana::label colourLbl{ *this, nana::rectangle(138, 87, 70, 20) };
	nana::combox colourCb{ *this, nana::rectangle(208, 85, 70, 20) };
	nana::label responseLbl{ *this, nana::rectangle(18, 113, 70, 20) };
	nana::textbox responseTb{ *this, nana::rectangle(78, 112, 200, 20) };
	nana::label statusLbl{ *this, nana::rectangle(18, 137, 270, 20) };
	nana::timer m_timer;
	nana::label versionLbl{ *this, nana::rectangle(formWidth - 40, formHeight - 30, 30, 20) };

	bool checkCallsign();
	bool checkTimer();
	void checkCluster();
	bool checkResponse();

	double m_timerCount;

	SDRunoPlugin_DXClusterUi & m_parent;
	IUnoPluginController & m_controller;
};
