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

#include "ModuleManager.h"

// Standard Includes

// Library Includes
#include <boost/function.hpp>
#include <boost/any.hpp>

// Local Includes
#include "../sharedbase/ModuleInterface.h"
#include "../sharedbase/EventLogger.h"
#include "../sharedbase/EntityList.h"
#include "../sharedbase/Envelope.h"
#include "../ScriptDLL/EntityFactory.h"

// Static class member initialization

// Class methods in the order they are defined within the class header


ModuleManager::ModuleManager( PropertyMap* gprops, MessageRouter* msgrouter, EntityList* emgr) : gprops(gprops), msgrouter(msgrouter), emgr(emgr) {

	std::shared_ptr<Subscriber> screate(new Subscriber(boost::bind(&ModuleManager::CreateEntity, this, _1)));
	this->msgrouter->Subscribe(CORE_MESSAGE::CREATE, screate);
	std::shared_ptr<Subscriber> sload(new Subscriber(boost::bind(&ModuleManager::LoadFromMessage, this, _1)));
	this->msgrouter->Subscribe(CORE_MESSAGE::LOADLIBRARY, sload);
	std::shared_ptr<Subscriber> suload(new Subscriber(boost::bind(&ModuleManager::UnloadFromMessage, this, _1)));
	this->msgrouter->Subscribe(1234, suload);
}

void ModuleManager::Load(std::string name) {
	char buf[256];
	
	if (this->libraries.find(name) == this->libraries.end()) {
		std::string libFileName = name;
		
		// Load the DLL ONLY if the libary has not already been loaded in the past.
#ifdef _WIN32
		libFileName += ".dll";
		HMODULE libdll = LoadLibrary(libFileName.c_str());
#else
		libFileName += ".so";
		void * libdll = dlopen(libFileName.c_str(), RTLD_LAZY);
#endif
		if (libdll != NULL) {
			this->libraries[name] = libdll;
			
			LOG(LOG_PRIORITY::FLOW, "Loaded library '" + name + "' successfully.");
		}
		else {
#ifdef _WIN32
			DWORD errcode = GetLastError();
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errcode, 0, buf, 256, NULL);
#else
			buf = "Error loading library: " + name;
#endif
			LOG(LOG_PRIORITY::RESTART, buf);
			
			LOG(LOG_PRIORITY::RESTART, "Module loading aborted due to error.");
			return; // *NOTE: This early return may be considered bad style, but was added to maintain modularity between the DLL loader code and the module facotry loading code.
		}
	}
	else {
		LOG(LOG_PRIORITY::CONFIG, "Library '" + name + "' already loaded, not reloading.");
	}

	{
		ModuleInstanceFactory fact;
#ifdef _WIN32
		fact = (ModuleInstanceFactory)GetProcAddress(this->libraries[name], "ModuleFactory");
#else
		fact = (ModuleInstanceFactory)dlsym(this->libraries[name], "ModuleFactory");
#endif

		if (fact != nullptr) {
			LOG(LOG_PRIORITY::FLOW, "Module factory acquired successfully.");
			
			ModuleInterface* module = fact(this->gprops, this->msgrouter, this->emgr, EventLogger::GetEventLogger());
			this->modules[name] = module;
		}
		else {
#ifdef _WIN32
			DWORD errcode = GetLastError();
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errcode, 0, buf, 256, NULL);
#else
			buf = "Error loading module factory";
#endif
			LOG(LOG_PRIORITY::RESTART, buf);
			
			LOG(LOG_PRIORITY::RESTART, "Module loading aborted due to error.");
			
			Unload(name);
			
			return;
		}
	}
}

void ModuleManager::Unload( std::string name /*= ""*/ ) {
	if (this->libraries.find(name) != this->libraries.end()) { // Lib WAS found
		if (this->modules.find(name) != this->modules.end()) { // Mod WAS found
			//Shutdown(name);
		}
		
#ifdef _WIN32
		if (FreeLibrary(this->libraries[name]) != 0) {
#else
		if (dlclose(this->libraries[name])) {
#endif
			this->libraries.erase(name);
		}
		else {
			LOG(LOG_PRIORITY::RESTART, "Unable to unload the library '" + name + "'!");
		}
	}
}

void ModuleManager::Update( double dt /*= 0.0f*/ ) {
	for (auto it = this->modules.begin(); it != this->modules.end(); ++it) {
		(*it).second->Update(dt);
	}
}

void ModuleManager::CreateEntity( EnvelopeSPTR e ) {
	EntitySPTR ent(boost::any_cast<EntitySPTR>(e->GetData(1)));
	std::map< std::string, std::map<std::string, boost::any> > comps = boost::any_cast<std::map< std::string, std::map<std::string, boost::any> > >(e->GetData(2));
		
	for (auto itr = this->modules.begin(); itr != this->modules.end(); ++itr) {
		for (auto itr2 = comps.begin(); itr2 != comps.end(); ++itr2) {
			(*itr).second->CreateComponent((*itr2).first, (*itr2).second, ent);
		}
	}
}

void ModuleManager::LoadFromMessage( EnvelopeSPTR e ) {
	int count = boost::any_cast<int>(e->GetData(0));
	for (int i = 0; i < count; ++i) {
		this->Load(boost::any_cast<std::string>(e->GetData(i+1)));
	}
}

void ModuleManager::UnloadFromMessage( EnvelopeSPTR e  ) {
	int count = boost::any_cast<int>(e->GetData(0));
	for (int i = 0; i < count; ++i) {
		this->Unload(boost::any_cast<std::string>(e->GetData(i+1)));
	}
}
void ModuleManager::Shutdown() {
	for (auto itr = this->modules.begin(); itr != this->modules.end(); ++itr) {
		LOG(LOG_PRIORITY::INFO, "Deleting module '" + itr->first + "'!");
		delete itr->second;
	}

	for (auto itr = this->libraries.begin(); itr != this->libraries.end(); ++itr) {
		LOG(LOG_PRIORITY::INFO, "Unloading library '" + itr->first + "'!");
#ifdef _WIN32
		if (FreeLibrary(this->libraries[itr->first]) != 0) {
#else
		if (dlclose(this->libraries[name])) {
#endif
		}
	}
}
