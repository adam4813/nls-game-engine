# -*- cmake -*-

if(DEFINED NLS_ENGINE_LIBRARY_MODE OR DEFINED NLS_ENGINE_BUILD_MODE)
	include(LoadLibraries) # Needed to get the function it provides, so that the LIB_AngelScript_DIR variable would be set and the AS library can be set up for building.

	# Get the main AS library
	load_library_source_from_web(
		"AngelScript"
		"AngelScript" # no trailing slash
		"http://www.angelcode.com/angelscript/sdk/files/angelscript_2.31.2.zip"
		"c4c604311de498c1beb3bf367edd2490"
	)

	# Get the extra addon
	load_library_source_from_web(
		"AngelScript_addon_aswrapper"
		"AngelScript/sdk/add_on" # no trailing slash
		"http://www.angelcode.com/angelscript/extras/aswrapper.zip"
		"2948d9b7da58267bb60cc69d8c405a71"
	)
endif(DEFINED NLS_ENGINE_LIBRARY_MODE OR DEFINED NLS_ENGINE_BUILD_MODE)

add_subdirectory(${LIB_AngelScript_DIR}/sdk/angelscript/projects/cmake)

## Inject addons into angelscript library build
foreach(loop_addon ${AS_ADDONS})
	file(GLOB ADDON_SOURCE_GLOB
		"${LIB_AngelScript_DIR}/sdk/add_on/${loop_addon}/as*.cpp"
		"${LIB_AngelScript_DIR}/sdk/add_on/${loop_addon}/script*.cpp"
	)
	set(AngelScript_ADDON_SOURCE ${AngelScript_ADDON_SOURCE} ${ADDON_SOURCE_GLOB})
	
	file(GLOB ADDON_HEADER_GLOB
		"${LIB_AngelScript_DIR}/sdk/add_on/${loop_addon}/as*.h"
		"${LIB_AngelScript_DIR}/sdk/add_on/${loop_addon}/script*.h"
	)
	set(AngelScript_ADDON_SOURCE_HEADERS ${AngelScript_ADDON_SOURCE_HEADERS} ${ADDON_HEADER_GLOB})
endforeach(loop_addon)

target_sources(${ANGELSCRIPT_LIBRARY_NAME} PRIVATE ${AngelScript_ADDON_SOURCE} ${AngelScript_ADDON_SOURCE_HEADERS})
## /Inject addons into angelscript library build

set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${LIB_AngelScript_DIR}/sdk/angelscript ${LIB_AngelScript_DIR}/sdk/add_on)

