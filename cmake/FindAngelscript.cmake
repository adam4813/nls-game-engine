#
#  ANGELSCRIPT_FOUND - angelscript was found
#  Angelscript_INCLUDE_DIRS - the angelscript library and addon include directories 
#  Angelscript_LIBRARIES - the angelscript library (including debug)
#

FIND_PATH(Angelscript_INCLUDE_DIRS angelscript.h
	PATHS
	/usr/local
	/usr
	PATH_SUFFIXES include
	)

FIND_PATH(Angelscript_ADDON_DIR scriptstdstring.h
	PATHS
	/usr/local
	/usr
	PATH_SUFFIXES scriptstdstring
	)

SET(Angelscript_NAMES ${Angelscript_NAMES} angelscript)

FIND_LIBRARY(Angelscript_LIBRARY_DEBUG
	NAMES ${Angelscript_NAMES}d
	PATHS
	/usr/local
	/usr
	PATH_SUFFIXES lib lib64 lib32
	)
FIND_LIBRARY(Angelscript_LIBRARY_RELEASE
	NAMES ${Angelscript_NAMES}
	PATHS
	/usr/local
	/usr
	PATH_SUFFIXES lib lib64 lib32
	)
IF (Angelscript_LIBRARY_DEBUG AND Angelscript_LIBRARY_RELEASE)
	SET(Angelscript_LIBRARY debug ${Angelscript_LIBRARY_DEBUG} optimized ${Angelscript_LIBRARY_RELEASE})
ELSEIF (Angelscript_LIBRARY_DEBUG AND NOT Angelscript_LIBRARY_RELEASE)
	SET(Angelscript_LIBRARY ${Angelscript_LIBRARY_DEBUG})
ELSEIF (NOT Angelscript_LIBRARY_DEBUG AND Angelscript_LIBRARY_RELEASE)
	SET(Angelscript_LIBRARY ${Angelscript_LIBRARY_RELEASE})
ELSE()
	SET(Anglescript_LIBRARY "")
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set ANGELSCRIPT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Angelscript DEFAULT_MSG Angelscript_LIBRARY Angelscript_INCLUDE_DIRS)

MARK_AS_ADVANCED(Angelscript_LIBRARY Angelscript_LIBRARIES Angelscript_INCLUDE_DIRS)