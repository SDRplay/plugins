#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/system/platform.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/widgets/progress.hpp>
#include <nana/threads/pool.hpp>
#include <nana/gui/dragger.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/paint/graphics.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)
#define formWidth (630)
#define formHeight (340)

class ADSBpluginUi;

class ADSBpluginForm : public nana::form
{

public:

	ADSBpluginForm(ADSBpluginUi& parent, IUnoPluginController& controller);
	~ADSBpluginForm();
	
	void Run();
	void SavePosition();
	void SetStartButtonState(bool state);
	bool GetStartButtonState();
	void SetStartButtonCaption(std::string text);
	void ClearDisplay();
	void PutDisplayString(char *str);
	void SetHttpPorts(const std::string& ports);
	void SetRawPorts(const std::string& ports);
	void SetSbsPorts(const std::string& ports);
	void SetBeastPorts(const std::string& ports);
	bool GetOverSample();
	void SetOverSample(bool en);
	std::string GetHttpPorts();
	std::string GetRawPorts();
	std::string GetSbsPorts();
	std::string GetBeastPorts();
	int GetSettingsX();
	int GetSettingsY();
	void SetSettingsX(int x);
	void SetSettingsY(int y);

private:

	void Setup();
	void GetStoredValues(int &x, int &y);
	bool GetOversampleSetting();
	void LoadSettings();
	void LoadPosition();

	int SettingsX = 0;
	int SettingsY = 0;

	int posX = 0;
	int posY = 0;

	// Set these two to be relative to the size of the overall form
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	// Added topBarHeight to accomodate new header style
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth) , formHeight - topBarHeight - bottomBarHeight) };
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
	nana::picture close_button{ *this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button{ *this, nana::rectangle(0, 0, 20, 15) };
	nana::label versionLbl{ *this, nana::rectangle(formWidth - 45, formHeight - 30, 35, 20) };

	// Settings panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();
	void SettingsDialog_Closed();

	// Help button
	nana::paint::image img_help_normal;
	nana::paint::image img_help_down;
	nana::picture help_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void HelpButton_Click();

	// Rest of UI form
	nana::button m_startButton{ *this, nana::rectangle(formWidth - 130, formHeight - 35, 80, 20) };
	nana::listbox m_aircraftLb{ *this, nana::rectangle(15, 35, formWidth - 30, formHeight - 80) };

	// Updates
	nana::timer m_updateTimer;
	std::list<std::string> m_updateItems;
	bool m_updateRequired;
	bool m_clearRequired;
	std::mutex m_updateLock;
	void TimerTick();

	ADSBpluginUi & m_parent;
	IUnoPluginController & m_controller;
};
