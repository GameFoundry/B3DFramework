cmake_minimum_required (VERSION 3.7.2)
project (Banshee)

# Version
set (BS_VERSION_MAJOR 0)
set (BS_VERSION_MINOR 4)

set (BS_PREBUILT_DEPENDENCIES_VERSION 2)
set (BS_SRC_DEPENDENCIES_VERSION 8)

# Configuration types
if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
	set(CMAKE_CONFIGURATION_TYPES "Debug;OptimizedDebug;Release;" CACHE INTERNAL "") 
else()
	if(NOT CMAKE_BUILD_TYPE)
		message("Defaulting to release build.")
		set(CMAKE_BUILD_TYPE Release CACHE INTERNAL "")
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
	# Set the valid options for cmake-gui drop-down list
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug OptimizedDebug Release)
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(BS_64BIT true)
endif()

# Options
set(AUDIO_MODULE "OpenAudio" CACHE STRING "Audio backend to use.")
set_property(CACHE AUDIO_MODULE PROPERTY STRINGS OpenAudio FMOD)

set(PHYSICS_MODULE "PhysX" CACHE STRING "Physics backend to use.")
set_property(CACHE PHYSICS_MODULE PROPERTY STRINGS PhysX)

set(INPUT_MODULE "OIS" CACHE STRING "Input backend to use.")
set_property(CACHE INPUT_MODULE PROPERTY STRINGS OIS)

if(WIN32)
set(RENDER_API_MODULE "DirectX 11" CACHE STRING "Render API to use.")
set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "DirectX 11" "OpenGL" "Vulkan")
else()
set(RENDER_API_MODULE "OpenGL" CACHE STRING "Render API to use.")
set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "OpenGL" "Vulkan")
endif()

set(RENDERER_MODULE "RenderBeast" CACHE STRING "Renderer backend to use.")
set_property(CACHE RENDERER_MODULE PROPERTY STRINGS RenderBeast)

set(BUILD_SCOPE "Runtime" CACHE STRING "Determines which parts of Banshee to build. Pick Framework to build only the low-level C++ framework. Pick Runtime to build everything, including the framework, scripting API and the editor.")
set_property(CACHE BUILD_SCOPE PROPERTY STRINGS "Runtime" "Framework")

set(INCLUDE_ALL_IN_WORKFLOW OFF CACHE BOOL "If true, all libraries (even those not selected) will be included in the generated workflow (e.g. Visual Studio solution). This is useful when working on engine internals with a need for easy access to all parts of it. Only relevant for workflow generators like Visual Studio or XCode.")

set(GENERATE_SCRIPT_BINDINGS ON CACHE BOOL "If true, script binding files will be generated. Script bindings are required for the project to build properly, however they take a while to generate. If you are sure the script bindings are up to date, you can turn off their generation (temporarily) to speed up the build.")

if(BUILD_SCOPE MATCHES "Runtime")
	set(BUILD_EDITOR ON)
else()
	set(BUILD_EDITOR OFF)
endif()

mark_as_advanced(CMAKE_INSTALL_PREFIX)

# Ensure dependencies are up to date
## Check prebuilt dependencies that user downloads in a .zip
set(BUILTIN_DEP_VERSION_FILE ${PROJECT_SOURCE_DIR}/../Dependencies/.version)
if(NOT EXISTS ${BUILTIN_DEP_VERSION_FILE})
	message(FATAL_ERROR "Precompiled dependencies package is missing our out of date. Re-run setup or manually download the latest dependencies package.")
else()
	file (STRINGS ${BUILTIN_DEP_VERSION_FILE} CURRENT_BUILTIN_DEP_VERSION)
	if(${BS_PREBUILT_DEPENDENCIES_VERSION} GREATER ${CURRENT_BUILTIN_DEP_VERSION})
		message(FATAL_ERROR "Your precomiled dependencies package is out of date. Re-run setup or manually download the latest dependencies package.")
	endif()
endif()
	
## Check dependencies built from source
set(SOURCE_DEP_BUILD_DIR ${PROJECT_SOURCE_DIR}/../Dependencies/Build)
set(SOURCE_DEP_BUILD_VERSION_FILE ${SOURCE_DEP_BUILD_DIR}/.version)
if(EXISTS ${SOURCE_DEP_BUILD_VERSION_FILE})
	file (STRINGS ${SOURCE_DEP_BUILD_VERSION_FILE} CURRENT_SOURCE_DEP_VERSION)
	if(${BS_SRC_DEPENDENCIES_VERSION} GREATER ${CURRENT_SOURCE_DEP_VERSION})
		message(STATUS "Built dependencies are out of date, forcing recompile.")
		execute_process(COMMAND "${CMAKE_COMMAND}" 
			-E remove_directory "${SOURCE_DEP_BUILD_DIR}")	
	endif()
else()
	message(STATUS "Built dependencies are out of date, forcing recompile.")
	execute_process(COMMAND "${CMAKE_COMMAND}" 
		-E remove_directory "${SOURCE_DEP_BUILD_DIR}")	
endif()

# Includes required for various find_package calls
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

# Global compile & linker flags
## Compiler-agnostic settings
### Target at least C++14
set(CMAKE_CXX_STANDARD 14)
## Compiler-specific settings
if(MSVC)
	# Linker
	set(BS_LINKER_FLAGS_COMMON "/DYNAMICBASE /NOLOGO")

	set(BS_LINKER_FLAGS_DEBUG "${BS_LINKER_FLAGS_COMMON} /DEBUG")
	set(BS_LINKER_FLAGS_OPTIMIZEDDEBUG "${BS_LINKER_FLAGS_COMMON} /LTCG:incremental /INCREMENTAL:NO /OPT:REF /DEBUG")
	set(BS_LINKER_FLAGS_RELEASE "${BS_LINKER_FLAGS_COMMON} /LTCG /INCREMENTAL:NO /OPT:REF")
	
	if(BS_64BIT)
		set(BS_LINKER_FLAGS_OPTIMIZEDDEBUG "${BS_LINKER_FLAGS_OPTIMIZEDDEBUG} /OPT:ICF")
		set(BS_LINKER_FLAGS_RELEASE "${BS_LINKER_FLAGS_RELEASE} /OPT:ICF")
	endif()
	
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DLL ${BS_LINKER_FLAGS_DEBUG}")
	set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/DLL ${BS_LINKER_FLAGS_DEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${BS_LINKER_FLAGS_DEBUG})
	
	set(CMAKE_SHARED_LINKER_FLAGS_OPTIMIZEDDEBUG "/DLL ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG}")
	set(CMAKE_MODULE_LINKER_FLAGS_OPTIMIZEDDEBUG "/DLL ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_OPTIMIZEDDEBUG ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG})
	
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DLL ${BS_LINKER_FLAGS_RELEASE}")
	set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DLL ${BS_LINKER_FLAGS_RELEASE}")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${BS_LINKER_FLAGS_RELEASE})
	
	# Compiler
	set(BS_COMPILER_FLAGS_COMMON "/GS- /W3 /GR- /WX- /nologo /bigobj /wd\"4577\" /wd\"4530\"")
	set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} /Gm /Od /RTC1 /MDd")
	
	if(BS_64BIT) # Debug edit and continue for 64-bit
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
	else() # Normal debug for 32-bit
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
	endif()
	
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} /GL /Gy /Zi /Gm /O2 /Oi /MD")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} /GL /Gy /O2 /Oi /MD")
	
	# Global defines
	#add_definitions(-D_HAS_EXCEPTIONS=0)
	
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# Note: Optionally add -ffunction-sections, -fdata-sections, but with linker option --gc-sections
	# TODO: Use link-time optimization -flto. Might require non-default linker.
	set(BS_COMPILER_FLAGS_COMMON "-fpic -fno-exceptions -fno-strict-aliasing -fno-rtti -fno-ms-compatibility -fms-extensions")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} -g -O0")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} -gline-tables-only -O2")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} -g0 -O2")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# TODO: Use link-time optimization -flto. Might require non-default linker.
	set(BS_COMPILER_FLAGS_COMMON "-fpic -fno-exceptions -fno-strict-aliasing -fno-rtti")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} -g -O0")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} -gline-tables-only -O2")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} -g0 -O2")

else()
# TODO_OTHER_COMPILERS_GO_HERE
endif()

# Output
if(BS_64BIT)
	set(BS_OUTPUT_DIR_PREFIX x64)
else()
	set(BS_OUTPUT_DIR_PREFIX x86)
endif()

set(BS_BINARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../bin/${BS_OUTPUT_DIR_PREFIX})
set(BS_LIBRARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../lib/${BS_OUTPUT_DIR_PREFIX})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BS_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_BINARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BS_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BS_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_BINARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BS_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BS_LIBRARY_OUTPUT_DIR}/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_LIBRARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${BS_LIBRARY_OUTPUT_DIR}/Release)

if(WIN32)
	set(BS_LIBRARY_EXTENSION ".lib")
else()
	set(BS_LIBRARY_EXTENSION ".a")
endif()

include(CMake/Common.cmake)
include(CMake/FindPackageOrBuild.cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

# Config file
## Note: Must happen before script binding generation
## Set names of libraries used in the config file
if(RENDER_API_MODULE MATCHES "DirectX 11")
	set(RENDER_API_MODULE_LIB BansheeD3D11RenderAPI)
elseif(RENDER_API_MODULE MATCHES "Vulkan")
	set(RENDER_API_MODULE_LIB BansheeVulkanRenderAPI)
else()
	set(RENDER_API_MODULE_LIB BansheeGLRenderAPI)
endif()

if(AUDIO_MODULE MATCHES "FMOD")
	set(AUDIO_MODULE_LIB BansheeFMOD)
else() # Default to OpenAudio
	set(AUDIO_MODULE_LIB BansheeOpenAudio)
endif()

set(RENDERER_MODULE_LIB RenderBeast)
set(INPUT_MODULE_LIB BansheeOISInput)
set(PHYSICS_MODULE_LIB BansheePhysX)

if(BUILD_EDITOR)
	set(BS_EDITOR_BUILD 1)
else()
	set(BS_EDITOR_BUILD 0)
endif()

## Generate config files)
configure_file("${PROJECT_SOURCE_DIR}/CMake/BsEngineConfig.h.in" "${PROJECT_SOURCE_DIR}/BansheeEngine/Include/BsEngineConfig.h")
configure_file("${PROJECT_SOURCE_DIR}/CMake/BsFrameworkConfig.h.in" "${PROJECT_SOURCE_DIR}/BansheeUtility/Include/BsFrameworkConfig.h")

# Code generation for script bindings
include(CMake/GenerateScriptBindings.cmake)
	
# Sub-directories
## Layers
add_subdirectory(BansheeUtility)
add_subdirectory(BansheeCore)
add_subdirectory(BansheeEngine)

if(BUILD_EDITOR OR MSVC)
	add_subdirectory(BansheeEditor)
endif()

## Plugins
### If using MSVC include all plugins in the solution (which ones actually build is controlled by dependencies)
if(INCLUDE_ALL_IN_WORKFLOW AND MSVC)
	add_subdirectory(BansheeD3D11RenderAPI)
	add_subdirectory(BansheeGLRenderAPI)
	add_subdirectory(BansheeVulkanRenderAPI)
	add_subdirectory(BansheeFMOD)
	add_subdirectory(BansheeOpenAudio)
else() # Otherwise include only chosen ones
	if(RENDER_API_MODULE MATCHES "DirectX 11")
		add_subdirectory(BansheeD3D11RenderAPI)
	elseif(RENDER_API_MODULE MATCHES "Vulkan")
		add_subdirectory(BansheeVulkanRenderAPI)
	else()
		add_subdirectory(BansheeGLRenderAPI)
	endif()

	if(AUDIO_MODULE MATCHES "FMOD")
		add_subdirectory(BansheeFMOD)
	else() # Default to OpenAudio
		add_subdirectory(BansheeOpenAudio)
	endif()
endif()

add_subdirectory(RenderBeast)
add_subdirectory(BansheeOISInput)
add_subdirectory(BansheePhysX)
add_subdirectory(BansheeFBXImporter)
add_subdirectory(BansheeFontImporter)
add_subdirectory(BansheeFreeImgImporter)
add_subdirectory(BansheeMono)
add_subdirectory(BansheeSL)

## Script interop
add_subdirectory(SBansheeEngine)

if(BUILD_EDITOR OR (INCLUDE_ALL_IN_WORKFLOW AND MSVC))
	add_subdirectory(SBansheeEditor)
endif()

## Executables
add_subdirectory(Examples/ExampleGettingStarted)
add_subdirectory(Examples/ExampleLowLevelRendering)
add_subdirectory(Examples/ExamplePhysicallyBasedShading)

if(BUILD_EDITOR OR (INCLUDE_ALL_IN_WORKFLOW AND MSVC))
	add_subdirectory(BansheeEditorExec)
	add_subdirectory(Game)
endif()

## Local experimental projects (not part of main repository)
add_subdirectory_optional(Experimental/PBS)

## Managed projects
if(MSVC)
	if(BUILD_EDITOR)
		include_external_msproject(MBansheeEngine ${PROJECT_SOURCE_DIR}/MBansheeEngine/MBansheeEngine.csproj)
		include_external_msproject(MBansheeEditor ${PROJECT_SOURCE_DIR}/MBansheeEditor/MBansheeEditor.csproj)
		
		set_property(TARGET MBansheeEngine PROPERTY FOLDER Script)
		set_property(TARGET MBansheeEditor PROPERTY FOLDER Script)
		
		add_dependencies(BansheeEditorExec MBansheeEngine MBansheeEditor)
		add_dependencies(Game MBansheeEngine)
		
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT BansheeEditorExec)
	endif()
else()
# TODO - Use Mono compiler to build the managed code as a pre-build step
endif()