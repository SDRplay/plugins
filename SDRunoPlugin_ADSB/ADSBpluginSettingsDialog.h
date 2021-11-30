#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#define dialogFormWidth (297)
#define dialogFormHeight (240)

class ADSBpluginForm;

class ADSBpluginSettingsDialog : public nana::form
{

public:

	ADSBpluginSettingsDialog(ADSBpluginForm& parent);
	~ADSBpluginSettingsDialog();

	void Run();

private:

	void Setup();

	nana::label m_http_portsLb{ *this, nana::rectangle(20, 20, 80, 20) };
	nana::label m_raw_portsLb{ *this, nana::rectangle(20, 60, 80, 20) };
	nana::label m_sbs_portsLb{ *this, nana::rectangle(20, 85, 80, 20) };
	nana::label m_beast_portsLb{ *this, nana::rectangle(20, 110, 80, 20) };
	nana::textbox m_http_portsTb{ *this, nana::rectangle(100, 20, 100, 20) };
	nana::textbox m_raw_portsTb{ *this, nana::rectangle(100, 60, 100, 20) };
	nana::textbox m_sbs_portsTb{ *this, nana::rectangle(100, 85, 100, 20) };
	nana::textbox m_beast_portsTb{ *this, nana::rectangle(100, 110, 100, 20) };
	nana::checkbox m_oversampleCkbox{ *this, nana::rectangle(20, 140, 100, 20) };

	ADSBpluginForm& m_parent;
};

