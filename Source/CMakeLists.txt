# Version
set (BS_FRAMEWORK_VERSION_MAJOR 1)
set (BS_FRAMEWORK_VERSION_MINOR 1)
set (BS_FRAMEWORK_VERSION_PATCH 0)
set (BS_FRAMEWORK_CURRENT_RELEASE_YEAR 2019)

set (BS_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION 11)
set (BS_FRAMEWORK_SRC_DEPENDENCIES_VERSION 1)
set (BS_FRAMEWORK_BUILTIN_ASSETS_VERSION 6)

# Options
set(AUDIO_MODULE "OpenAudio" CACHE STRING "Audio backend to use.")
set_property(CACHE AUDIO_MODULE PROPERTY STRINGS OpenAudio FMOD Null)

set(PHYSICS_MODULE "PhysX" CACHE STRING "Physics backend to use.")
set_property(CACHE PHYSICS_MODULE PROPERTY STRINGS "PhysX" "Null")

# include imgui plugin will be default false.
set(INCLUDE_IMGUI_PLUGIN "IncludeImguiPlugin" CACHE BOOL "Include BsfImgui submodule plugin")

if(WIN32)
	set(RENDER_API_MODULE "DirectX 11" CACHE STRING "Render API to use.")
	set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "DirectX 11" "OpenGL" "Vulkan" "Null")
elseif(APPLE)
	set(RENDER_API_MODULE "OpenGL" CACHE STRING "Render API to use.")
	set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "OpenGL" "Vulkan" "Null")
else()
	set(RENDER_API_MODULE "OpenGL" CACHE STRING "Render API to use.")
	set_property(CACHE RENDER_API_MODULE PROPERTY STRINGS "OpenGL" "Vulkan" "Null")
endif()

set(RENDERER_MODULE "RenderBeast" CACHE STRING "Renderer backend to use.")
set_property(CACHE RENDERER_MODULE PROPERTY STRINGS RenderBeast Null)

set(INCLUDE_ALL_IN_WORKFLOW OFF CACHE BOOL "If true, all libraries (even those not selected) will be included in the generated workflow (e.g. Visual Studio solution). This is useful when working on engine internals with a need for easy access to all parts of it. Only relevant for workflow generators like Visual Studio or XCode.")

set(BUILD_TESTS OFF CACHE BOOL "If true, build targets for running unit tests will be included in the output.")

set(BUILD_BSL OFF CACHE BOOL "If true, build lexer & parser for BSL. Requires flex & bison dependencies.")

set(BUILD_ALL_RENDER_API OFF CACHE BOOL "If true, all supported render backends will be built, regardless of choice in RENDER_API_MODULE. Choice in RENDER_API_MODULE will still be used as the default.")

set(INCLUDE_ASSET_PACKAGING_SCRIPTS OFF CACHE BOOL "If true, generate helper targets that allow you to import built-in assets as well as upload new package versions.")

set(UPLOAD_ASSETS OFF CACHE BOOL "Only relevant if BUILD_ASSETS is true. When enabled the system will upload a package with the latest assets whenever assets are built, provided valid server and credentials are present.")

set(ENABLE_COTIRE false CACHE BOOL "Enable cotire's precompiled headers and unity build support (experimental).")

set(EXPERIMENTAL_ENABLE_NETWORKING OFF CACHE BOOL "If true, enable experimental networking support.")

# Add cotire if enabled
if(ENABLE_COTIRE)
	include(${BSF_SOURCE_DIR}/CMake/cotire.cmake)
endif()

set(USE_BUNDLED_LIBRARIES ON CACHE BOOL "Use and install bundled libraries")

# Ensure dependencies are up to date
## Check prebuilt dependencies that are downloaded in a .zip
check_and_update_binary_deps(bsfDependencies Dependencies ${BSF_SOURCE_DIR}/../Dependencies/ ${BS_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION})

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
check_and_update_builtin_assets(bsf ${BSF_SOURCE_DIR}/../Data/Raw Raw ${BS_FRAMEWORK_BUILTIN_ASSETS_VERSION} NO)

# Config file
## Note: Must happen before script binding generation
## Set names of libraries used in the config file
if(RENDER_API_MODULE MATCHES "DirectX 11")
	set(RENDER_API_MODULE_LIB bsfD3D11RenderAPI)
elseif(RENDER_API_MODULE MATCHES "Vulkan")
	set(RENDER_API_MODULE_LIB bsfVulkanRenderAPI)
elseif(RENDER_API_MODULE MATCHES "OpenGL")
	set(RENDER_API_MODULE_LIB bsfGLRenderAPI)
else()
	set(RENDER_API_MODULE_LIB bsfNullRenderAPI)
endif()

if(AUDIO_MODULE MATCHES "FMOD")
	set(AUDIO_MODULE_LIB bsfFMOD)
elseif(AUDIO_MODULE MATCHES "OpenAudio")
	set(AUDIO_MODULE_LIB bsfOpenAudio)
else()
	set(AUDIO_MODULE_LIB bsfNullAudio)
endif()

if(PHYSICS_MODULE MATCHES "PhysX")
	set(PHYSICS_MODULE_LIB bsfPhysX)
else()
	set(PHYSICS_MODULE_LIB bsfNullPhysics)
endif()

if(RENDERER_MODULE MATCHES "RenderBeast")
	set(RENDERER_MODULE_LIB bsfRenderBeast)
else()
	set(RENDERER_MODULE_LIB bsfNullRenderer)
endif()

if((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR BS_IS_BANSHEE3D)
	set(BS_SCRIPTING_ENABLED 1)
else()
	set(BS_SCRIPTING_ENABLED 0)
endif()

## Generate config files
configure_file("${BSF_SOURCE_DIR}/CMake/BsEngineConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/bsfEngine/BsEngineConfig.h")
configure_file("${BSF_SOURCE_DIR}/CMake/BsFrameworkConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/bsfUtility/BsFrameworkConfig.h")

# Set files disallowed in precompiled headers
set_directory_properties(PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Sub-directories
## Foundation
add_subdirectory(Foundation)

## Plugins
### If using MSVC include all plugins in the solution (which ones actually build is controlled by dependencies)
if(INCLUDE_ALL_IN_WORKFLOW AND MSVC)
	add_subdirectory(Plugins/bsfD3D11RenderAPI)
	add_subdirectory(Plugins/bsfGLRenderAPI)
	add_subdirectory(Plugins/bsfVulkanRenderAPI)
	add_subdirectory(Plugins/bsfNullRenderAPI)
	add_subdirectory(Plugins/bsfFMOD)
	add_subdirectory(Plugins/bsfOpenAudio)
	add_subdirectory(Plugins/bsfNullAudio)
	add_subdirectory(Plugins/bsfPhysX)
	add_subdirectory(Plugins/bsfNullPhysics)
	add_subdirectory(Plugins/bsfRenderBeast)
	add_subdirectory(Plugins/bsfNullRenderer)
	
	## Scripting
	add_subdirectory(Scripting/bsfMono)
	add_subdirectory(Scripting/bsfScript)
	add_subdirectory(Scripting/bsfSharp)
else() # Otherwise include only chosen ones
	if(BUILD_ALL_RENDER_API)
		if(WIN32)
			add_subdirectory(Plugins/bsfD3D11RenderAPI)
		endif()
		
		if(NOT APPLE)
			add_subdirectory(Plugins/bsfVulkanRenderAPI)
		endif()
		
		add_subdirectory(Plugins/bsfGLRenderAPI)
		add_subdirectory(Plugins/bsfNullRenderAPI)
	else()
		if(RENDER_API_MODULE MATCHES "DirectX 11")
			add_subdirectory(Plugins/bsfD3D11RenderAPI)
		elseif(RENDER_API_MODULE MATCHES "Vulkan")
			add_subdirectory(Plugins/bsfVulkanRenderAPI)
		elseif(RENDER_API_MODULE MATCHES "OpenGL")
			add_subdirectory(Plugins/bsfGLRenderAPI)
		else()
			add_subdirectory(Plugins/bsfNullRenderAPI)
		endif()
	endif()

	if(AUDIO_MODULE MATCHES "FMOD")
		add_subdirectory(Plugins/bsfFMOD)
	elseif(AUDIO_MODULE MATCHES "OpenAudio")
		add_subdirectory(Plugins/bsfOpenAudio)
	else()
		add_subdirectory(Plugins/bsfNullAudio)
	endif()
	
	if(PHYSICS_MODULE MATCHES "PhysX")
		add_subdirectory(Plugins/bsfPhysX)
	else()
		add_subdirectory(Plugins/bsfNullPhysics)
	endif()
	
	if(RENDERER_MODULE MATCHES "RenderBeast")
		add_subdirectory(Plugins/bsfRenderBeast)
	else()
		add_subdirectory(Plugins/bsfNullRenderer)
	endif()
	
	if((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR BS_IS_BANSHEE3D)
		add_subdirectory(Scripting/bsfScript)
	endif()
	
	if((SCRIPT_API AND SCRIPT_API MATCHES "C#") OR BS_IS_BANSHEE3D)
		add_subdirectory(Scripting/bsfMono)
		add_subdirectory(Scripting/bsfSharp)
	endif()
endif()

if (INCLUDE_IMGUI_PLUGIN)
	# if true, then git clone the imgui plugin (with recursive submodules). it
	# will be cloned into the build directory where it will be configured as
	# bsfImgui target. other targets can then set bsfImgui as their dependency
	# in their respective cmake files.
	include(CMake/Plugins/BsImgui.cmake)
endif()

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

	add_executable(CoreTest
		Foundation/bsfCore/Private/UnitTests/BsCoreTest.cpp)
		
	target_link_libraries(CoreTest bsf)
	
	set_property(TARGET UtilityTest PROPERTY FOLDER Tests)
	set_property(TARGET CoreTest PROPERTY FOLDER Tests)	
	
	add_test(NAME UtilityTests COMMAND $<TARGET_FILE:UtilityTest>)
	add_test(NAME CoreTests COMMAND $<TARGET_FILE:UtilityTest>)
endif()

## Builtin resource preprocessing
add_executable(bsfImportTool
	Foundation/bsfEngine/Resources/BsBuiltinResourcesImporter.cpp)
add_common_flags(bsfImportTool)
target_compile_definitions(bsfImportTool PRIVATE -DBS_IS_ASSET_TOOL)

target_link_libraries(bsfImportTool bsf)
	
add_engine_dependencies(bsfImportTool)
add_importer_dependencies(bsfImportTool)

set_property(TARGET bsfImportTool PROPERTY FOLDER Utilities)

if(NOT BS_IS_BANSHEE3D)
	install(TARGETS bsfImportTool RUNTIME DESTINATION bin)	
endif()
	
set(BS_FTP_CREDENTIALS_FILE "${PROJECT_SOURCE_DIR}/../ftp_credentials" CACHE STRING "The location containing the FTP server credentials to use for uploading packages. The file is expected to contain three lines: URL/Username/Password, in that order.")
mark_as_advanced(BS_FTP_CREDENTIALS_FILE)

if(${INCLUDE_ASSET_PACKAGING_SCRIPTS})
	add_run_asset_import_target(bsfBuiltinAssets ${BSF_SOURCE_DIR}/../Data ${BSF_SOURCE_DIR}/.. "")
	add_upload_assets_target(bsfBuiltinAssets "bsfCompiledData_Master" ${BSF_SOURCE_DIR}/../Data Raw/DataPackageContents.txt)
	add_upload_assets_target(bsfDocImages "bsfDocImagesData_Master" ${BSF_SOURCE_DIR}/../Documentation DataPackageContents.txt)
endif()

set(COMPILED_ASSETS_VERSION_FILE ${BSF_SOURCE_DIR}/../Data/.reqversion)
file (STRINGS ${COMPILED_ASSETS_VERSION_FILE} COMPILED_ASSETS_VERSION)
check_and_update_builtin_assets(bsfCompiled ${BSF_SOURCE_DIR}/../Data Data ${COMPILED_ASSETS_VERSION} NO)

set(DOC_IMAGES_VERSION_FILE ${BSF_SOURCE_DIR}/../Documentation/.reqversion)
file (STRINGS ${DOC_IMAGES_VERSION_FILE} DOC_IMAGES_VERSION)
check_and_update_builtin_assets(bsfDocImages ${BSF_SOURCE_DIR}/../Documentation Documentation ${DOC_IMAGES_VERSION} NO)

## Script binding generation
if(((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR BS_IS_BANSHEE3D) AND SCRIPT_BINDING_GENERATION)
	include(${BSF_SOURCE_DIR}/CMake/GenerateScriptBindings.cmake)
endif()

if(NOT BS_IS_BANSHEE3D)
	set(DATA_DST_DIR bin)
else()
	set(DATA_DST_DIR .)
endif()

## Install
install(
	DIRECTORY ../Data
	DESTINATION ${DATA_DST_DIR}
	PATTERN "../Data/Raw" EXCLUDE
	PATTERN "../Data/Shaders/Includes" EXCLUDE
	PATTERN ".version" EXCLUDE
	PATTERN ".reqversion" EXCLUDE
)

install(
	DIRECTORY ../Data/Raw/Shaders/Includes
	DESTINATION ${DATA_DST_DIR}/Data/Shaders
)
