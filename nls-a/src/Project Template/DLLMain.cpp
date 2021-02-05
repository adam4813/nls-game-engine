#define __dll__

/**
* \file file base name
* \author Firstname Lastname
* \date YYYY-MM-DD
* \brief  Module description
*
*
*
*/

// Standard Includes

// Library Includes
#include <boost/bind.hpp>
#include <windows.h>

// Local Includes
#include "../SharedBase/MessageDispatcher.h"
#include "../sharedbase/EventLogger.h"
#include "Module.h" // *TODO change me to the current module's include

// Forward Declarations
class GlobalProperties;
class ModuleInterface;

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*) {
	return true;
}

__declspec(dllexport) ModuleInterface* ModuleFactory(GlobalProperties* gprops, MsgFunc msgfunc)
{
	EventLogger::module = "Project Template"; // *TODO change me to the current module's name
	return new module(gprops, msgfunc); // *TODO change me to the current module's instance
}