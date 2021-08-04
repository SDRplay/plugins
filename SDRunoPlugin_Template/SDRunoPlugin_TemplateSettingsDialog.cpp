#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "SDRunoPlugin_TemplateSettingsDialog.h"
#include "SDRunoPlugin_TemplateUi.h"
#include "resource.h"
#include <io.h>
#include <shlobj.h>

// Form constructor with handles to parent and uno controller - launches form TemplateForm
SDRunoPlugin_TemplateSettingsDialog::SDRunoPlugin_TemplateSettingsDialog(SDRunoPlugin_TemplateUi& parent, IUnoPluginController& controller) :
	nana::form(nana::API::make_center(dialogFormWidth, dialogFormHeight), nana::appearance(true, false, true, false, false, false, false)),
	m_parent(parent),
	m_controller(controller)
{
	Setup();	
}

// Form deconstructor
SDRunoPlugin_TemplateSettingsDialog::~SDRunoPlugin_TemplateSettingsDialog()
{
	// **This Should not be necessary, but just in case - we are going to remove all event handlers
	// previously assigned to the "destroy" event to avoid memory leaks;
	this->events().destroy.clear();
}

// Start Form and start Nana UI processing
void SDRunoPlugin_TemplateSettingsDialog::Run()
{	
	show();
	nana::exec();
}

int SDRunoPlugin_TemplateSettingsDialog::LoadX()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TemplateSettings.X", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int SDRunoPlugin_TemplateSettingsDialog::LoadY()
{
	std::string tmp;
	m_controller.GetConfigurationKey("TemplateSettings.Y", tmp);
	if (tmp.empty())
	{
		return -1;
	}
	return stoi(tmp);
}

// Create the settings dialog form
void SDRunoPlugin_TemplateSettingsDialog::Setup()
{
	// TODO: Form code starts here

	// Load X and Y locations for the dialog from the ini file (if exists)
	int posX = LoadX();
	int posY = LoadY();
	move(posX, posY);

	// This code sets the plugin size and title
	size(nana::size(dialogFormWidth, dialogFormHeight));
	caption("SDRuno Plugin Template - Settings");

	// Set the forms back color to black to match SDRuno's settings dialogs
	this->bgcolor(nana::colors::black);

	// TODO: Extra form code goes here	
}