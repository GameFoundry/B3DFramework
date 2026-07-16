cmake_minimum_required (VERSION 3.31.0)

if(CMAKE_HOST_APPLE)
	if(NOT CMAKE_OSX_ARCHITECTURES)
		set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "macOS target architecture" FORCE)
	elseif(NOT CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
		message(FATAL_ERROR "Banshee targets Apple Silicon only. Set CMAKE_OSX_ARCHITECTURES=arm64.")
	endif()

	if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
		set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0 CACHE STRING "Minimum macOS deployment version" FORCE)
	elseif(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 13.0)
		message(FATAL_ERROR "Banshee's Apple Silicon Metal target requires macOS 13.0 or newer.")
	endif()
endif()

# CSharp is only supported by the Visual Studio generators.
if(CMAKE_GENERATOR MATCHES "Visual Studio")
	set(B3D_PROJECT_LANGUAGES C CXX CSharp)
else()
	set(B3D_PROJECT_LANGUAGES C CXX)
endif()
project (bsf LANGUAGES ${B3D_PROJECT_LANGUAGES})

set (B3D_FRAMEWORK_ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set (B3D_FRAMEWORK_SOURCE_FOLDER ${B3D_FRAMEWORK_ROOT_FOLDER}/Source)
set (APP_ROOT_DIR ${PROJECT_SOURCE_DIR})
set (B3D_IS_ENGINE 0)

# Default install dir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	if(LINUX)
		set(CMAKE_INSTALL_PREFIX /usr/local/opt/b3d CACHE PATH "..." FORCE)
	endif()
endif()

# Options
set(B3D_SCRIPT_API "None" CACHE STRING "Which scripting API/language to use, if any. If no scripting API is chosen only native C++ bsf core will be built. If a scripting API is chosen, then a separate scripting layer on top of the core will be included in the build as well.")
set_property(CACHE B3D_SCRIPT_API PROPERTY STRINGS "None" "C#")

set(B3D_STRIP_DEBUG_INFO ON CACHE BOOL "If true debug info will be stripped into a separate file in release builds.")

set(B3D_ENABLE_EXCEPTIONS OFF CACHE BOOL "If true C++ exceptions will be enabled when compiling.")

set(B3D_ENABLE_RTTI OFF CACHE BOOL "If true C++ RTTI will be enabled when compiling.")

# Common includes
include(Source/CMake/Prerequisites.cmake)

# Subdirectories
add_subdirectory(Source)
B3DRegisterOptionalFrameworkSubdirectories()

