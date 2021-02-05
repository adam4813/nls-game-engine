include(ExternalProject)
find_package(GLM QUIET)

if(GLFW_FOUND)
	message(STATUS "Found GLM")
else()
	message(STATUS "GLM not found - will build from source")

	ExternalProject_Add(glm SOURCE_DIR ${LIBS_SOURCE_PATH}/glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		CONFIGURE_COMMAND ";" BUILD_COMMAND ";" INSTALL_COMMAND ";"
		LOG_DOWNLOAD 1
	)

	ExternalProject_Get_Property(glm SOURCE_DIR)
	set(GLM_INCLUDE_DIRS ${SOURCE_DIR})
endif()

set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIRS} CACHE STRING "")