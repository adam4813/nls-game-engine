// *HACK We will disable the warning about no standard extension for scoped enums, as this is valid c++11
// #pragma warning (disable : 4482) // Done through project property page

#include <cassert>
#include <set>

#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <EngineConfig.h>

#include "../windows/win32.h"

#include "../sharedbase/OSInterface.h"
#include "../sharedbase/EntityList.h"
#include "../sharedbase/EventLogger.h"
#include "../sharedbase/PropertyMap.h"
#include "../sharedbase/MessageRouter.h"
#include "../sharedbase/Envelope.h"
#include "ModuleManager.h"

/// Only to help clean the parsing out of the lexer in main.
#define TRY_CONFIG_PARSE(dest, type, index) {try {dest = boost::any_cast<type>(config->GetData(index));} catch (boost::bad_any_cast&) {operating_system->ShowWarning(NLS_I18N::CONFIG_PARSE_VALUE_ERROR + key); valid = false; continue;} catch (std::out_of_range&) {operating_system->ShowWarning(NLS_I18N::CONFIG_PARSE_VALUE_MISSING_ERROR + key); valid = false; continue;}}

struct RequireFlags {
	RequireFlags(bool required) :
		required(required),
		multiple_allowed(false),
		found(false)
		{
	}
	
	RequireFlags(bool required, bool multiple_allowed) :
		required(required),
		multiple_allowed(multiple_allowed),
		found(false)
		{
	}
	
	bool required;
	bool multiple_allowed;
	bool found;
};

typedef std::map<std::string, RequireFlags> KeyRequiredMap;
typedef KeyRequiredMap::value_type KeyRequired;

int main() {
	std::string window_title, log_file_name = NLS_ENGINE_DEFAULT_LOG_FILE, userspace_game_path;
	unsigned int window_width, window_height;
	
	EventLogger* elog = EventLogger::GetEventLogger();
	EventLogger::module = "Main";

	// Create all of our shared managers
	PropertyMap* gprops = new PropertyMap();
	MessageRouter* msgrtr = new MessageRouter();
	EntityList* emgr = new EntityList();
	ModuleManager modmgr(gprops, msgrtr, emgr);

	gprops->SetProperty("UserGameFolder", "..");

	// Load the OS interface *TODO: make this available to anything that might want it.  Maybe a gprop?  Header file with a global function?
	OSInterfaceSPTR operating_system(new win32(gprops, msgrtr));
	
	// Load the system configuration from disk
	// *TODO: either make a serializable property tree (and replace property map's usage with it too) or make propertymap serializable.  Either way, use such here instead of the envelope.
	{
		EnvelopeSPTR config(new Envelope());
		if (!LoadFromDisk(config, NLS_ENGINE_CONFIG_PATH)) {
#ifndef _DEBUG
			operating_system->ShowError(NLS_I18N::CONFIG_LOAD_ERROR);
			
			return 1;
#else
			operating_system->ShowInfo("Config data not found, creating from hard-coded values.");
			
			config->AddData(std::string("WindowTitle"));
			config->AddData(std::string("Game"));
			config->AddData(std::string("WindowSize"));
			config->AddData(800u);
			config->AddData(600u);
			config->AddData(std::string("LogFile"));
			config->AddData(std::string("Game.log"));
			config->AddData(std::string("UserGameFolder"));
			config->AddData(std::string("NLS Game"));
			
			SaveToDisk(config, NLS_ENGINE_CONFIG_PATH);
#endif
		}
		
		// Collect the valid keys
		KeyRequiredMap keys;
		keys.insert(KeyRequired("WindowTitle", RequireFlags(true)));
		keys.insert(KeyRequired("WindowSize", RequireFlags(true)));
		keys.insert(KeyRequired("UserGameFolder", RequireFlags(false)));
		keys.insert(KeyRequired("LogFile", RequireFlags(false)));
		
		// Parse and lex the loaded config file
		bool valid = true;
		std::string key;
		for (unsigned int index = 0; valid && index < config->GetCount(); ++index) {
			try {
				key = boost::any_cast<std::string>(config->GetData(index));
			}
			catch (boost::bad_any_cast&) {
				operating_system->ShowWarning(NLS_I18N::CONFIG_PARSE_KEY_ERROR);
				valid = false;
				continue; // Escape the loop so as to protect the following code from the error case.
			}
			
			KeyRequiredMap::iterator iter = keys.find(key);
			if (iter != keys.end()) {
				if (!iter->second.found || iter->second.multiple_allowed) {
					iter->second.found = true;
					
					if (key == "WindowTitle") {
						TRY_CONFIG_PARSE(window_title, std::string, ++index);
						LOG(LOG_PRIORITY::FLOW, "Config: Window title read as: " + window_title);
					}
					else if (key == "WindowSize") {
						TRY_CONFIG_PARSE(window_width, unsigned int, ++index);
						TRY_CONFIG_PARSE(window_height, unsigned int, ++index);
						LOG(LOG_PRIORITY::FLOW, "Config: Window size read as: <" + boost::lexical_cast<std::string>(window_width) + ", " + boost::lexical_cast<std::string>(window_height) + ">");
					}
					else if (key == "LogFile") {
						TRY_CONFIG_PARSE(log_file_name, std::string, ++index);
						
						LOG(LOG_PRIORITY::FLOW, "Config: Log file name read as: " + log_file_name);
					}
					else if (key == "UserGameFolder") {
						TRY_CONFIG_PARSE(userspace_game_path, std::string, ++index);
						
						userspace_game_path = operating_system->GetPath(DIR_USER) + "/" + userspace_game_path;
						
						bool dir_exists = false;
						try {
							dir_exists = boost::filesystem::exists(userspace_game_path) && boost::filesystem::is_directory(userspace_game_path);
						}
						catch (boost::filesystem::filesystem_error& err) {
							operating_system->ShowError(NLS_I18N::FILESYSTEM_PER_USER_FOLDER_ACCESS_ERROR + err.what(), NLS_I18N::FILESYSTEM_ERROR_TITLE);
							valid = false;
						}
						
						if (valid) {
							if (dir_exists) {
								// Attempt to modify it to see if it is writeable.
								std::ofstream out_stream;
								out_stream.open(userspace_game_path + "/FfSDFadf.txt", std::ios_base::out | std::ios_base::app);
								if (!out_stream.bad()) {
									out_stream << "test";
									out_stream.close();
									boost::filesystem::remove(userspace_game_path + "/FfSDFadf.txt");
								}
								else {
									operating_system->ShowError(NLS_I18N::FILESYSTEM_PER_USER_FOLDER_WRITE_ERROR, NLS_I18N::FILESYSTEM_ERROR_TITLE);
									valid = false;
								}
							}
							else {
								// Create it
								LOG(LOG_PRIORITY::FLOW, "Creating per-user game configuration folder: " + userspace_game_path);
								
								try {
									boost::filesystem::create_directories(userspace_game_path);
								}
								catch (boost::filesystem::filesystem_error& err) {
									operating_system->ShowError(NLS_I18N::FILESYSTEM_PER_USER_FOLDER_CREATE_ERROR + err.what(), NLS_I18N::FILESYSTEM_ERROR_TITLE);
									valid = false;
								}
							}
							
							if (valid) {
								gprops->SetProperty("UserGameFolder", userspace_game_path);
								LOG(LOG_PRIORITY::FLOW, "Config: Per-user game folder file path read as: " + userspace_game_path);
							}
						}
					}
					else {
						assert(false /* Missing an if-statement! */);
					}
				}
				else {
					operating_system->ShowWarning(NLS_I18N::CONFIG_LEX_DUP_KEY_ERROR + key);
					valid = false;
				}
			}
			else {
				operating_system->ShowWarning(NLS_I18N::CONFIG_LEX_KEY_ERROR + key);
				valid = false;
			}
		}
		
		// Verify that all required keys were found
		BOOST_FOREACH(const KeyRequired& pair, keys) {
			if (pair.second.required && !pair.second.found) {
				operating_system->ShowWarning(NLS_I18N::CONFIG_KEY_MISSING_ERROR + pair.first);
				valid = false;
			}
		}
		
		if (!valid) {
			return 1;
		}
	}

	// Set up the message logger with the file
	elog->SetLogFile(userspace_game_path + "/" + log_file_name); // *NOTE: After this point it is safe to log to disk without notifying the user of error conditions.
	LOG(LOG_PRIORITY::FLOW, "Log file created!");

	// Start the message thread
	boost::thread msgThread(&MessageRouter::Route, msgrtr);

	// Create our main window and store the handle the global properties
	HWND hwnd = boost::any_cast<HWND>(operating_system->CreateGUIWindow(window_width, window_height, window_title, WINDOW_INNER_SIZE));
	gprops->SetProperty("hwnd", hwnd);
	
	// Start the library manager and load all cores
	modmgr.Load("scriptdll");

	// Send the startup message to all modules
	{
		EnvelopeSPTR e(new Envelope);
		e->msgid = CORE_MESSAGE::STARTUP;
		msgrtr->SendSP(e, false);
	}

	// Timing variables used in the update function for the main loop.
	operating_system->SetupTimer();
	double elapsed = 0.0f;

	while (1) {
		operating_system->RouteMessages();

		elapsed = operating_system->GetElapsedTime();

		// Calls update for each core.
		if (msgThread.timed_join( boost::posix_time::time_duration(0,0,0,0)) == true) {
			break;
		}

		modmgr.Update(elapsed);
	}

	// Send the shutdown message to all the modules
	{
		EnvelopeSPTR e(new Envelope);
		e->msgid = CORE_MESSAGE::SHUTDOWN;
		msgrtr->SendSP(e, false);
	}
	
	delete emgr;
	delete msgrtr;
	delete gprops;
	modmgr.Shutdown();
	delete elog;

	return 0;
}
