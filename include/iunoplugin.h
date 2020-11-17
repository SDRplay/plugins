#pragma once

#ifdef _WIN32
#define ADD_EXPORTS
#ifdef ADD_EXPORTS

#define UNOPLUGINAPI __declspec(dllexport)
#else
#define UNOPLUGINAPI __declspec(dllimport)
#endif
#define UNOPLUGINCALL __cdecl
#else
#define UNOPLUGINAPI
#define UNOPLUGINCALL
#endif

#define UNOPLUGINAPIVERSION (0x00000002)

class IUnoPluginController;
class UnoEvent;

typedef unsigned short channel_t;

class IUnoPlugin
{

public:

	IUnoPlugin(IUnoPluginController& controller) :
		m_controller(controller)
	{

	}
	
	virtual ~IUnoPlugin()
	{

	}

	virtual const char* GetPluginName() const { return "Untitled Plugin"; }	

	virtual void HandleEvent(const UnoEvent& ev) { }
	virtual bool IsEnabled() { return true;  }

protected:

	IUnoPluginController& m_controller;
};
