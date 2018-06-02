# Version
set (BS_FRAMEWORK_VERSION_MAJOR 1)
set (BS_FRAMEWORK_VERSION_MINOR 0)

set (BS_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION 5)
set (BS_FRAMEWORK_SRC_DEPENDENCIES_VERSION 1)
set (BS_FRAMEWORK_BUILTIN_ASSETS_VERSION 4)

# Options
set(AUDIO_MODULE "OpenAudio" CACHE STRING "Audio backend to use.")
set_property(CACHE AUDIO_MODULE PROPERTY STRINGS OpenAudio FMOD)

set(PHYSICS_MODULE "PhysX" CACHE STRING "Physics backend to use.")
set_property(CACHE PHYSICS_MODULE PROPERTY STRINGS PhysX)

if(WIN32)
	set(RENDER_API_MODULE "DirectX 11" CACHE STRING "Render API to use.")
	set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "DirectX 11" "OpenGL" "Vulkan")
elseif(APPLE)
	set(RENDER_API_MODULE "OpenGL" CACHE STRING "Render API to use.")
	set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "OpenGL")
else()
	set(RENDER_API_MODULE "OpenGL" CACHE STRING "Render API to use.")
	set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "OpenGL" "Vulkan")
endif()

set(RENDERER_MODULE "RenderBeast" CACHE STRING "Renderer backend to use.")
set_property(CACHE RENDERER_MODULE PROPERTY STRINGS RenderBeast)

set(INCLUDE_ALL_IN_WORKFLOW OFF CACHE BOOL "If true, all libraries (even those not selected) will be included in the generated workflow (e.g. Visual Studio solution). This is useful when working on engine internals with a need for easy access to all parts of it. Only relevant for workflow generators like Visual Studio or XCode.")

set(BUILD_TESTS OFF CACHE BOOL "If true, build targets for running unit tests will be included in the output.")

set(BUILD_BSL OFF CACHE BOOL "If true, build lexer & parser for BSL. Requires flex & bison dependencies.")

# Ensure dependencies are up to date
## Check prebuilt dependencies that are downloaded in a .zip
check_and_update_binary_deps(bsf ${BSF_SOURCE_DIR}/../Dependencies/ ${BS_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION})

## Check dependencies built from source
if(WIN32)
	set(SOURCE_DEP_BUILD_DIR ${BSF_SOURCE_DIR}/../Dependencies/Build)
	set(SOURCE_DEP_BUILD_VERSION_FILE ${SOURCE_DEP_BUILD_DIR}/.version)
	if(EXISTS ${SOURCE_DEP_BUILD_VERSION_FILE})
		file (STRINGS ${SOURCE_DEP_BUILD_VERSION_FILE} CURRENT_SOURCE_DEP_VERSION)
		if(${BS_FRAMEWORK_SRC_DEPENDENCIES_VERSION} GREATER ${CURRENT_SOURCE_DEP_VERSION})
			message(STATUS "Built dependencies are out of date, forcing recompile.")
			execute_process(COMMAND "${CMAKE_COMMAND}" 
				-E remove_directory "${SOURCE_DEP_BUILD_DIR}")	
		endif()
	else()
		message(STATUS "Built dependencies are out of date, forcing recompile.")
		execute_process(COMMAND "${CMAKE_COMMAND}" 
			-E remove_directory "${SOURCE_DEP_BUILD_DIR}")	
	endif()
endif()

## Check data dependencies
check_and_update_builtin_assets(bsf ${BSF_SOURCE_DIR}/../Data ${BS_FRAMEWORK_BUILTIN_ASSETS_VERSION} YES)

# Config file
## Note: Must happen before script binding generation
## Set names of libraries used in the config file
if(RENDER_API_MODULE MATCHES "DirectX 11")
	set(RENDER_API_MODULE_LIB bsfD3D11RenderAPI)
elseif(RENDER_API_MODULE MATCHES "Vulkan")
	set(RENDER_API_MODULE_LIB bsfVulkanRenderAPI)
else()
	set(RENDER_API_MODULE_LIB bsfGLRenderAPI)
endif()

if(AUDIO_MODULE MATCHES "FMOD")
	set(AUDIO_MODULE_LIB bsfFMOD)
else() # Default to OpenAudio
	set(AUDIO_MODULE_LIB bsfOpenAudio)
endif()

set(RENDERER_MODULE_LIB bsfRenderBeast)
set(PHYSICS_MODULE_LIB bsfPhysX)

## Generate config files)
configure_file("${BSF_SOURCE_DIR}/CMake/BsEngineConfig.h.in" "${BSF_SOURCE_DIR}/Foundation/bsfEngine/BsEngineConfig.h")
configure_file("${BSF_SOURCE_DIR}/CMake/BsFrameworkConfig.h.in" "${BSF_SOURCE_DIR}/Foundation/bsfUtility/BsFrameworkConfig.h")

# Sub-directories
## Foundation
add_subdirectory(Foundation)

## Plugins
### If using MSVC include all plugins in the solution (which ones actually build is controlled by dependencies)
if(INCLUDE_ALL_IN_WORKFLOW AND MSVC)
	add_subdirectory(Plugins/bsfD3D11RenderAPI)
	add_subdirectory(Plugins/bsfGLRenderAPI)
	add_subdirectory(Plugins/bsfVulkanRenderAPI)
	add_subdirectory(Plugins/bsfFMOD)
	add_subdirectory(Plugins/bsfOpenAudio)
else() # Otherwise include only chosen ones
	if(RENDER_API_MODULE MATCHES "DirectX 11")
		add_subdirectory(Plugins/bsfD3D11RenderAPI)
	elseif(RENDER_API_MODULE MATCHES "Vulkan")
		add_subdirectory(Plugins/bsfVulkanRenderAPI)
	else()
		add_subdirectory(Plugins/bsfGLRenderAPI)
	endif()

	if(AUDIO_MODULE MATCHES "FMOD")
		add_subdirectory(Plugins/bsfFMOD)
	else() # Default to OpenAudio
		add_subdirectory(Plugins/bsfOpenAudio)
	endif()
endif()

add_subdirectory(Plugins/bsfRenderBeast)
add_subdirectory(Plugins/bsfPhysX)
add_subdirectory(Plugins/bsfFBXImporter)
add_subdirectory(Plugins/bsfFontImporter)
add_subdirectory(Plugins/bsfFreeImgImporter)
add_subdirectory(Plugins/bsfSL)

## Tests
if(BUILD_TESTS)
	enable_testing()

	add_executable(UtilityTest 
		Foundation/bsfUtility/Private/UnitTests/BsUtilityTest.cpp 
		Foundation/bsfUtility/Private/UnitTests/BsUtilityTestSuite.cpp
		Foundation/bsfUtility/Private/UnitTests/BsFileSystemTestSuite.cpp)
		
	target_link_libraries(UtilityTest bsf)
	target_include_directories(UtilityTest PRIVATE 
		"Foundation/bsfUtility"
		"Foundation/bsfUtility/ThirdParty")
	
	set_property(TARGET UtilityTest PROPERTY FOLDER Tests)

	add_test(NAME FrameworkTests COMMAND $<TARGET_FILE:UtilityTest>)
endif()

## Install
install(
	DIRECTORY ../Data
	DESTINATION bin/
	PATTERN "../Data/Raw" EXCLUDE
)
