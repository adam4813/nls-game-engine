#pragma once

/**
* \file file base name
* \author Adam Martin
* \date 2011-10-23
* \brief A manager class to load and start modules.
*
* A manager that can load/unload and start/stop modules at runtime through dynamically loaded
* libraries. The use of a common interface ModuleInterface allows us to have a uniform loading
* and starting procedure.
*/

// Standard Includes
#include <string>
#include <map>

// Library Includes
#include <boost/foreach.hpp> // Quick simple looping

// Local Includes
#include "../sharedbase/MessageRouter.h"
#include "../sharedbase/Envelope_fwd.h"

// Forward Declarations
class PropertyMap;
class ModuleInterface;
class EntityList;
class EventLogger;

// Typedefs
typedef ModuleInterface* (*ModuleInstanceFactory)(PropertyMap*, MessageRouter*, EntityList*, EventLogger*); // Used to find the address of the create system function
// Preprocessor selection based on OS
#ifdef _WIN32
#include <Windows.h>
typedef HMODULE DLLHANDLE;
#else
#include <dlfcn.h>
typedef void* DLLHANDLE;
#endif

class ModuleManager {
public:
	ModuleManager(PropertyMap* gprops, MessageRouter* msgrouter, EntityList* emgr);
	void Shutdown(); // Shuts down and removes all modules and their libraries.

	void Load(std::string name); // The name is required in order to load the new library
	void LoadFromMessage(EnvelopeSPTR e); // The name is required in order to load the new library
	void Unload(std::string name = "");
	void UnloadFromMessage(EnvelopeSPTR e);

	void Update(double dt = 0.0f);

	void CreateEntity(EnvelopeSPTR e);

private:
	PropertyMap* gprops;
	MessageRouter* msgrouter;
	EntityList* emgr;

	std::map<std::string, ModuleInterface*> modules; // A mapping of each core to its given name
	std::map<std::string, DLLHANDLE> libraries; // A mapping of each loaded library to a given filename
};
