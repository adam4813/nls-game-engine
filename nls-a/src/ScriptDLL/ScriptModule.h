#pragma once

#define NOMINMAX
#define AS_USE_FLOAT 1

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
#include <map>
#include <set>
#include <queue>
#include <utility>

// Library Includes
#include <windows.h>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <angelscript/scriptany/ScriptAny.h>
#include <angelscript/scriptmath/scriptmath.h>
#include <threading.h>
#include <EngineConfig.h>

// Local Includes
#include "../sharedbase/ModuleInterface.h"
#include "ScriptManager.h"
#include "../sharedbase/MessageRouter.h"
#include "../sharedbase/Envelope_fwd.h"
#include "../sharedbase/Entity_fwd.h"

// Forward Declarations
class PropertyMap;
class CreateEntityFactory;
class EntityList;

// Typedefs
typedef int                          event_handler_type;
typedef unsigned int                 event_handler_key_type;
typedef std::set<event_handler_type> event_handler_list_type;
typedef std::map<event_handler_key_type, event_handler_list_type> event_handler_map_type;

extern "C"
{
	__declspec(dllexport) ModuleInterface* ModuleFactory(PropertyMap*, MessageRouter*, EntityList* emgr, EventLogger*);
};

static const std::string STARTUP_SCRIPT = NLS_ENGINE_DATA_PATH + "/scripts/main.as";

CreateEntityFactory* CreateEntityFactoryFact(void);

void Log(int priority, std::string string);

class ScriptModule : public ModuleInterface {
public:
	ScriptModule(PropertyMap* gprops, MessageRouter* msgrouter, EntityList* emgr );

	~ScriptModule();

	void SendMessage(EnvelopeSPTR envelope) const; // Send a message with a shared pointer envelope
	void PrioritySend( EnvelopeSPTR envelope ) const;
	EnvelopeSPTR DirectedSend( EnvelopeSPTR envelope ) const;

	void Message(EnvelopeSPTR envelope);

	/**
	* Alias for RegisterMessageHandlerByName
	*/
	unsigned int RegisterMessageHandler(std::string funcName, event_handler_key_type eventID);
	
	/**
	* Finds the function ID by name and registers it via RegisterMessageHandlerByID
	*/
	unsigned int RegisterMessageHandlerByName(std::string funcName, event_handler_key_type eventID);
	
	/**
	* Finds the function ID by the function's declaration and registers it via RegisterMessageHandlerByID
	*/
	unsigned int RegisterMessageHandlerByDecl(std::string funcDeclaration, event_handler_key_type eventID);

	/**
	* Registers the given function ID to the given event ID
	*/
	unsigned int RegisterMessageHandlerByID( unsigned int funcID, event_handler_key_type eventID );

	/**
	* Alias for UnregisterMessageHandlerByName
	*/
	unsigned int UnregisterMessageHandler(std::string funcName, event_handler_key_type eventID);
	
	/**
	* Finds the function ID by name and unregisters it via UnregisterMessageHandlerByID
	*/
	unsigned int UnregisterMessageHandlerByName(std::string funcName, event_handler_key_type eventID);
	
	/**
	* Finds the function ID by the function's declaration and unregisters it via UnregisterMessageHandlerByID
	*/
	unsigned int UnregisterMessageHandlerByDecl(std::string funcDeclaration, event_handler_key_type eventID);

	/**
	* Unregisters the given function ID from the given event ID
	*/
	unsigned int UnregisterMessageHandlerByID( unsigned int funcID, event_handler_key_type eventID );

	// Wrapper template for shared pointer behavior.
	template <typename Type>
	struct SharedPointerWrapper {
		// Constructor wrapper.
		static void construct(void* memory) {
			new(memory) std::shared_ptr<Type>();
		}
		// Destructor wrapper.
		static void destruct(void* memory) {
			((std::shared_ptr<Type>*)memory)->~shared_ptr();
		}
	};

	template <typename Type>
	void RegisterSharedPointer(const std::string&, const std::string&);

	EnvelopeSPTR EnvelopeSPTRFactory(); // Factory used by AS to get an instance of EnvelopeSPTR

	void CreateComponent(std::string, std::map< std::string, boost::any > &, EntitySPTR);
	WHO_DELETES::TYPE RemoveComponent(ComponentInterface*);

	void Startup(EnvelopeSPTR envelope);

	void Shutdown(EnvelopeSPTR envelope);

	void Update(double dt);


	/**
	* Alias for RegisterUpdateHandlerByName
	*/
	unsigned int RegisterUpdateHandler(std::string funcName);
	
	/**
	* Finds the function ID by name and registers it via RegisterUpdateHandlerByID
	*/
	unsigned int RegisterUpdateHandlerByName(std::string funcName);
	
	/**
	* Registers a function for a one-time call next update event
	*/
	unsigned int RegisterUpdateHandlerByID(unsigned int funcID);


private:
	ScriptManager updateScriptManager;
	ScriptManager eventScriptManager;

	double deltaAccumulator; // Accumulator for the change in time between each call to update

	mutable Threading::ReadWriteMutex eventHandlersMutex;
	event_handler_map_type eventHandlers;
	
	mutable Threading::ReadWriteMutex updateHandlersMutex;
	std::queue<unsigned int> updateHandlers;
};
