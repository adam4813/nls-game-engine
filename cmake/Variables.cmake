# -*- cmake -*-
#
# Definitions of variables used throughout the NLS Engine build process.
#


# Relative and absolute paths to subtrees.

# Set up the destinations for the compiled output
set(EXECUTABLE_OUTPUT_PATH ${NLS_ENGINE_ROOT}/bin)
set(LIBRARY_OUTPUT_PATH ${NLS_ENGINE_ROOT}/lib)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${LIBRARY_OUTPUT_PATH})

set(LIBS_DOWNLOAD_PATH ${NLS_ENGINE_ROOT}/lib_archives CACHE PATH "Location of library source archives.")
mark_as_advanced(FORCE LIBS_DOWNLOAD_PATH)

set(LIBS_SOURCE_PATH ${NLS_ENGINE_ROOT}/lib_src CACHE PATH "Location of libraries' sourcecode.")
mark_as_advanced(FORCE LIBS_SOURCE_PATH)

set(LIBS_BINARY_PATH ${LIBRARY_OUTPUT_PATH} CACHE PATH "Location of built library binaries.")
mark_as_advanced(FORCE LIBS_BINARY_PATH)

## Correct the build output location
# First for the generic no-config case (e.g. with mingw)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
# Second, for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${EXECUTABLE_OUTPUT_PATH})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBRARY_OUTPUT_PATH})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBRARY_OUTPUT_PATH})
endforeach(OUTPUTCONFIG)

# Define the angelscript addons wanted for the build
set(AS_ADDONS autowrapper scriptany scriptarray scriptbuilder scriptdictionary scriptmath scriptstdstring aswrapper)
#aswrapper
#autowrapper
#contextmgr
#debugger
#scriptany
#scriptarray
#scriptbuilder
#scriptdictionary
#scriptfile
#scripthandle
#scripthelper
#scriptmath
#scriptstdstring
#serializer

