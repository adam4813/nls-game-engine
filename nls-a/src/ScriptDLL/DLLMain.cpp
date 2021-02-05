#define __dll__
// *HACK We will disable the warning about no standard extension for scoped enums, as this is valid c++11
// #pragma warning (disable : 4482) // Done through project property page

/**
* \file file base name
* \author Adam Martin
* \date 2011-07-20
* \brief  ComponentInterface class used as a common base for all components.
*
* The use of a common interface allows all components to be acted on in the same manner, and provides
* a guaranteed way of interacting with that component.
*/

// Standard Includes

// Library Includes
#include <boost/bind.hpp>
#include <windows.h>

// Local Includes
#include "../sharedbase/EventLogger.h"
#include "ScriptModule.h" // *TODO change me to the current module's include

// Forward Declarations
class PropertyMap;
class ModuleInterface;
class MessageRouter;
class EntityList;

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*) {
	return true;
}

__declspec(dllexport) ModuleInterface* ModuleFactory(PropertyMap* gprops, MessageRouter* msgrouter, EntityList* emgr = nullptr, EventLogger* elog = nullptr)
{
	EventLogger::module = "Script";
	EventLogger::SetEventLogger(elog);
	return new ScriptModule(gprops, msgrouter, emgr);
}
