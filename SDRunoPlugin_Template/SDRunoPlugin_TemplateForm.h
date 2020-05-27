#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (25)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
#define formWidth (297)
#define formHeight (240 - topBarHeight)

class SDRunoPlugin_TemplateUi;

class SDRunoPlugin_TemplateForm : public nana::form
{

public:

	SDRunoPlugin_TemplateForm(SDRunoPlugin_TemplateUi& parent, IUnoPluginController& controller);		
	~SDRunoPlugin_TemplateForm();
	
	void Run();
	
private:

	void Setup();

	// Set these two to be relative to the size of the overall form
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, 0, formWidth - (2 * sideBorderWidth), formHeight - bottomBarHeight) };

	// TODO: Add UI controls here

	SDRunoPlugin_TemplateUi & m_parent;
	IUnoPluginController & m_controller;
};
