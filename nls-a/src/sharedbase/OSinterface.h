#pragma once

/*
	Author: Adam Martin
	Date: 2011-12-22
	Description: Base interface for any OS specific code
*/

// Standard Includes
#include <string>
#include <memory>

// Library Includes
#include <boost/any.hpp>
#include <EngineConfig.h>

// Local Includes

// Forward Declarations
class PropertyMap;
class MessageRouter;

// Typedefs
class OSInterface;
typedef std::shared_ptr<OSInterface> OSInterfaceSPTR;

/// Enumeration of known paths.  This provides an OS abstraction.
enum DIRS {
	DIR_USER, ///< On Linux: "$HOME". On Windows XP: SHGetFolderPath(CSIDL_APPDATA). On Windows Vista (or newer): SHGetKnownFolderPath(FOLDERID_RoamingAppData). on Mac: 
	DIR_DOCUMENTS, ///< On Linux: . On Windows XP: SHGetFolderPath(CSIDL_MYDOCUMENTS). On Windows Vista (or newer): SHGetKnownFolderPath(FOLDERID_Documents). on Mac: 
	DIR_PICTURES, ///< On Linux: . On Windows XP: SHGetFolderPath(CSIDL_MYPICTURES). On Windows Vista (or newer): SHGetKnownFolderPath(FOLDERID_Pictures). on Mac: 
	DIR_MUSIC, ///< On Linux: . On Windows XP: SHGetFolderPath(CSIDL_MYMUSIC). On Windows Vista (or newer): SHGetKnownFolderPath(FOLDERID_Music). on Mac: 
	DIR_VIDEO, ///< On Linux: . On Windows XP: SHGetFolderPath(CSIDL_MYVIDEO). On Windows Vista (or newer): SHGetKnownFolderPath(FOLDERID_Videos). on Mac: 
	DIR_DESKTOP, ///< On Linux: . On Windows XP: SHGetFolderPath(CSIDL_DESKTOPDIRECTORY). On Windows Vista (or newer): SHGetKnownFolderPath(FOLDERID_Desktop). on Mac: 
	DIR_1234 ///< nothing, just a placeholder. DO NOT USE.
};

enum WINDOW_FLAGS {
	WINDOW_OUTER_SIZE = 0, ///< Window should be sized by the outer boundry.
	WINDOW_INNER_SIZE = 1, ///< Window should be sized by the inside ("client") boundry.
	WINDOW_1234 ///< nothing, just a placeholder. DO NOT USE.
};

class OSInterface {
public:
	OSInterface( PropertyMap* gprops, MessageRouter* msgrouter ) : gprops(gprops), msgrouter(msgrouter) { }
	~OSInterface() {}
	
	/// Creates the platform specific window and returns the handle as a boost::any
	virtual boost::any CreateGUIWindow(int, int, std::string, WINDOW_FLAGS = WINDOW_OUTER_SIZE) = 0;
	
	/// Displays a message alerting the user to some non-critical information.  Displays a confirmation button.  Parameters are: body text, window title.  DOES NOT LOG TO DISK.
	virtual void ShowInfo(std::string, std::string = NLS_I18N::TITLE_INFO) = 0;
	
	/// Displays a message alerting the user to a non-critical problem.  Displays a confirmation button.  Parameters are: body text, window title.  DOES NOT LOG TO DISK.
	virtual void ShowWarning(std::string, std::string = NLS_I18N::TITLE_WARNING) = 0;
	
	/// Displays a message alerting the user to a critical problem.  Displays a confirmation button.  Parameters are: body text, window title.  DOES NOT LOG TO DISK.
	virtual void ShowError(std::string, std::string = NLS_I18N::TITLE_CRITICAL) = 0;
	
	/// Handles message routing from the OS to the respective functions or methods
	virtual void RouteMessages() = 0;
	
	/// Returns the path to a known system- or user-specific location.
	virtual std::string GetPath(DIRS) = 0;
	
	virtual void SetupTimer() = 0;
	virtual double GetElapsedTime() = 0;
protected:
	PropertyMap* gprops;
	MessageRouter* msgrouter;
};
