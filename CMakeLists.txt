cmake_minimum_required (VERSION 3.12.0)
project (bsf)

set (BSF_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Source)
set (APP_ROOT_DIR ${PROJECT_SOURCE_DIR})
set (BS_IS_BANSHEE3D 0)

# Default install dir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	if(LINUX)
		set(CMAKE_INSTALL_PREFIX /usr/local/opt/bsf CACHE PATH "..." FORCE)
	endif()
endif()

include(Source/CMake/Properties.cmake)
include(Source/CMake/FindPackageOrBuild.cmake)
include(Source/CMake/HelperMethods.cmake)

add_subdirectory(Source)

# Options
set(SCRIPT_API "None" CACHE STRING "Which scripting API/language to use, if any. If no scripting API is chosen only native C++ bsf core will be built. If a scripting API is chosen, then a separate scripting layer on top of the core will be included in the build as well.")
set_property(CACHE SCRIPT_API PROPERTY STRINGS "None" "C#")

set(SCRIPT_BINDING_GENERATION OFF CACHE BOOL "If true, script binding generation will be supported through a specialized build target. Enable this if you plan on modifying the scripting API. Requires the SBGen tool dependency. Only relevant if you have selected a SCRIPT_API other than \"None\".")

set(BSF_STRIP_DEBUG_INFO ON CACHE BOOL "If true debug info will be stripped into a separate file in release builds.")

set(BSF_ENABLE_EXCEPTIONS OFF CACHE BOOL "If true C++ exceptions will be enabled when compiling.")

set(BSF_ENABLE_RTTI OFF CACHE BOOL "If true C++ RTTI will be enabled when compiling.")
