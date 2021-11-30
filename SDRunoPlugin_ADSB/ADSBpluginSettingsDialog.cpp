#include <sstream>

#include "ADSBpluginSettingsDialog.h"
#include "ADSBpluginForm.h"
#include "resource.h"
#include <io.h>

// Form constructor with handles to parent - launches form ADSBpluginForm
ADSBpluginSettingsDialog::ADSBpluginSettingsDialog(ADSBpluginForm& parent) :
	nana::form(nana::API::make_center(dialogFormWidth, dialogFormHeight), nana::appearance(true, false, true, false, false, false, false)),
	m_parent(parent)
{
	Setup();
}

// Form deconstructor
ADSBpluginSettingsDialog::~ADSBpluginSettingsDialog()
{
	// **This Should not be necessary, but just in case - we are going to remove all event handlers
	// previously assigned to the "destroy" event to avoid memory leaks;
	this->events().destroy.clear();
}

// Start Form and start Nana UI processing
void ADSBpluginSettingsDialog::Run()
{
	show();
	nana::exec();
}

// Create the settings dialog form
void ADSBpluginSettingsDialog::Setup()
{
	// TODO: Form code starts here

	m_http_portsTb.caption(m_parent.GetHttpPorts());
	m_raw_portsTb.caption(m_parent.GetRawPorts());
	m_sbs_portsTb.caption(m_parent.GetSbsPorts());
	m_beast_portsTb.caption(m_parent.GetBeastPorts());
	m_oversampleCkbox.check(m_parent.GetOverSample());

	m_http_portsTb.events().text_changed([&] {m_parent.SetHttpPorts(m_http_portsTb.text()); });
	m_raw_portsTb.events().text_changed([&] {m_parent.SetRawPorts(m_raw_portsTb.text()); });
	m_sbs_portsTb.events().text_changed([&] {m_parent.SetSbsPorts(m_sbs_portsTb.text()); });
	m_beast_portsTb.events().text_changed([&] {m_parent.SetBeastPorts(m_beast_portsTb.text()); });
	m_oversampleCkbox.events().click([&] {m_parent.SetOverSample(m_oversampleCkbox.checked()); });

	// Load X and Y locations for the dialog from the ini file (if exists)
	move(m_parent.GetSettingsX(), m_parent.GetSettingsY());

	events().move([&](const nana::arg_move& mov) {
		m_parent.SetSettingsX(mov.x);
		m_parent.SetSettingsY(mov.y);
		});

	// This code sets the plugin size and title
	size(nana::size(dialogFormWidth, dialogFormHeight));
	caption("ADSB - Settings");

	// Set the forms back color to black to match SDRuno's settings dialogs
	this->bgcolor(nana::colors::black);

	m_http_portsLb.fgcolor(nana::colors::white);
	m_http_portsLb.transparent(true);
	m_http_portsLb.caption("HTTP Port:");
	m_raw_portsLb.fgcolor(nana::colors::white);
	m_raw_portsLb.transparent(true);
	m_raw_portsLb.caption("Raw Data Port:");
	m_sbs_portsLb.fgcolor(nana::colors::white);
	m_sbs_portsLb.transparent(true);
	m_sbs_portsLb.caption("SBS Data Port:");
	m_beast_portsLb.fgcolor(nana::colors::white);
	m_beast_portsLb.transparent(true);
	m_beast_portsLb.caption("Beast Data Port:");
	m_oversampleCkbox.caption("Oversample");
	m_oversampleCkbox.check(m_parent.GetOverSample());
	m_oversampleCkbox.transparent(true);
	m_oversampleCkbox.fgcolor(nana::colors::white);
}