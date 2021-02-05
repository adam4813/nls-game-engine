#pragma once
/*
	Author: Adam Martin
	Date: 2011-12-22
	Description: Win32 and windows specific code
*/

// Standard Includes
#include <string>

// Library Includes
#include <boost/any.hpp>
#include <windows.h>
#include <EngineConfig.h>

// Local Includes
#include "../sharedbase/OSinterface.h"
#include "../sharedbase/Envelope_fwd.h"

// Forward Declarations
class PropertyMap;
class MessageRouter;

class win32 : public OSInterface {
public:
	win32( PropertyMap* gprops, MessageRouter* msgrouter ) : OSInterface(gprops, msgrouter), freq(0.0f), elapsed(0.0f) { }
	~win32() {
		PostQuitMessage(0);
	}
	virtual boost::any CreateGUIWindow(int, int, std::string, WINDOW_FLAGS = WINDOW_OUTER_SIZE);
	virtual void ShowInfo(std::string, std::string = NLS_I18N::TITLE_INFO);
	virtual void ShowWarning(std::string, std::string = NLS_I18N::TITLE_WARNING);
	virtual void ShowError(std::string, std::string = NLS_I18N::TITLE_CRITICAL);
	virtual void RouteMessages();
	virtual std::string GetPath(DIRS);
	virtual void SetupTimer();
	virtual double GetElapsedTime();
	void Quit(EnvelopeSPTR e);

	void SetClientRect(int width, int height);

	static LRESULT CALLBACK Proc(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
private:
	MSG msg;
	LARGE_INTEGER count;
	double freq, elapsed;
	HWND handle;
};
