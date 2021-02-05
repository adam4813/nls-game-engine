/**
* \file DLLMain
* \author Ricky Curtice
* \date 2011-10-30
* \brief Basic entry point for the DLL system
*
*/
#define __dll__

// Standard Includes

// Library Includes
#include <boost/bind.hpp>
#include <windows.h>

// Local Includes
#include "../sharedbase/MessageRouter.h"
#include "../sharedbase/EventLogger.h"
#include "RenderModule.h"

// Forward Declarations
class PropertyMap;
class ModuleInterface;
class EntityList;

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*) {
	return true;
}

__declspec(dllexport) ModuleInterface* ModuleFactory(PropertyMap* gprops, MessageRouter* msgrouter, EntityList* emgr = nullptr, EventLogger* elog = nullptr)
{
	EventLogger::module = "DirectX Rendering Engine";
	EventLogger::SetEventLogger(elog);
	return new GraphicsCore::RenderModule(gprops, msgrouter);
}
