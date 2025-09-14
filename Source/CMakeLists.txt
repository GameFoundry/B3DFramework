# Version
set (B3D_FRAMEWORK_VERSION_MAJOR 1)
set (B3D_FRAMEWORK_VERSION_MINOR 1)
set (B3D_FRAMEWORK_VERSION_PATCH 0)
set (B3D_FRAMEWORK_CURRENT_RELEASE_YEAR 2019)

set (B3D_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION 12)
set (B3D_FRAMEWORK_SOURCE_DEPENDENCIES_VERSION 1)
set (B3D_FRAMEWORK_BUILTIN_ASSETS_VERSION 7)

# Options
set(B3D_AUDIO_BACKEND "OpenAudio" CACHE STRING "Audio backend to use.")
set_property(CACHE B3D_AUDIO_BACKEND PROPERTY STRINGS OpenAudio FMOD Null)

set(B3D_PHYSICS_BACKEND "PhysX" CACHE STRING "Physics backend to use.")
set_property(CACHE B3D_PHYSICS_BACKEND PROPERTY STRINGS "PhysX" "Null")

if(WIN32)
	set(B3D_RENDER_BACKEND "Vulkan" CACHE STRING "Render API to use.")
	set_property(CACHE B3D_RENDER_BACKEND PROPERTY STRINGS "DirectX 11" "OpenGL" "Vulkan" "Null")
elseif(APPLE)
	set(B3D_RENDER_BACKEND "Vulkan" CACHE STRING "Render API to use.")
	set_property(CACHE B3D_RENDER_BACKEND PROPERTY STRINGS "OpenGL" "Vulkan" "Null")
else()
	set(B3D_RENDER_BACKEND "Vulkan" CACHE STRING "Render API to use.")
	set_property(CACHE B3D_RENDER_BACKEND PROPERTY STRINGS "OpenGL" "Vulkan" "Null")
endif()

set(B3D_RENDERER "RenderBeast" CACHE STRING "Renderer backend to use.")
set_property(CACHE B3D_RENDERER PROPERTY STRINGS RenderBeast Null)

set(B3D_INCLUDE_ALL_IN_WORKFLOW OFF CACHE BOOL "If true, all libraries (even those not selected) will be included in the generated workflow (e.g. Visual Studio solution). This is useful when working on engine internals with a need for easy access to all parts of it. Only relevant for workflow generators like Visual Studio or XCode.")

set(B3D_BUILD_TESTS OFF CACHE BOOL "If true, build targets for running unit tests will be included in the output.")

set(B3D_BUILD_BSL_PARSER OFF CACHE BOOL "If true, build lexer & parser for BSL. Requires flex & bison dependencies.")

set(B3D_BUILD_ALL_RENDER_BACKENDS OFF CACHE BOOL "If true, all supported render backends will be built, regardless of choice in B3D_RENDER_BACKEND. Choice in B3D_RENDER_BACKEND will still be used as the default.")

set(B3D_BUILD_TYPE "Development" CACHE STRING "Determines the purpose of the build. 'Development' contains features to make debugging easier, but has overhead compared to other types - this should be your build type in most cases. 'Shipping' strips all development features.")
set_property(CACHE B3D_BUILD_TYPE PROPERTY STRINGS Development Shipping)

set(B3D_INCLUDE_ASSET_PACKAGING_SCRIPTS OFF CACHE BOOL "If true, generate helper targets that allow you to import built-in assets as well as upload new package versions.")

set(B3D_UPLOAD_ASSETS OFF CACHE BOOL "Only relevant if BUILD_ASSETS is true. When enabled the system will upload a package with the latest assets whenever assets are built, provided valid server and credentials are present.")

set(B3D_USE_DOTNETCORE ON CACHE BOOL "If enabled, uses new .NET Core 9.0 with support for C# 12. Otherwise uses .NET Framework 4.5 with support for C# 5.")

set(B3D_ENABLE_COTIRE false CACHE BOOL "Enable cotire's precompiled headers and unity build support (experimental).")

set(B3D_EXPERIMENTAL_ENABLE_NETWORKING OFF CACHE BOOL "If true, enable experimental networking support.")

# Add cotire if enabled
if(B3D_ENABLE_COTIRE)
	include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/cotire.cmake)
endif()

set(B3D_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Use and install bundled libraries")

# Ensure dependencies are up to date
## Check prebuilt dependencies that are downloaded in a .zip
check_and_update_binary_deps(bsfDependencies Dependencies ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/ ${B3D_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION})

## Check dependencies built from source
if(WIN32)
	set(sourceDependencyBuildFolder ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/Build)
	set(sourceDependencyBuildVersionFile ${sourceDependencyBuildFolder}/.version)
	if(EXISTS ${sourceDependencyBuildVersionFile})
		file (STRINGS ${sourceDependencyBuildVersionFile} currentSourceDependencyVersion)
		if(${B3D_FRAMEWORK_SOURCE_DEPENDENCIES_VERSION} GREATER ${currentSourceDependencyVersion})
			message(STATUS "Built dependencies are out of date, forcing recompile.")
			execute_process(COMMAND "${CMAKE_COMMAND}"
				-E remove_directory "${sourceDependencyBuildFolder}")
		endif()
	else()
		message(STATUS "Built dependencies are out of date, forcing recompile.")
		execute_process(COMMAND "${CMAKE_COMMAND}"
			-E remove_directory "${sourceDependencyBuildFolder}")
	endif()
endif()

## Check data dependencies
B3DUpdateBuiltinAssetsIfNeeded(bsf ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Data/Raw Raw ${B3D_FRAMEWORK_BUILTIN_ASSETS_VERSION})

# Config file
## Note: Must happen before script binding generation
## Set names of libraries used in the config file
if(B3D_RENDER_BACKEND MATCHES "DirectX 11")
	set(RENDER_API_MODULE_LIB bsfD3D11RenderAPI)
elseif(B3D_RENDER_BACKEND MATCHES "Vulkan")
	set(RENDER_API_MODULE_LIB bsfVulkanRenderAPI)
elseif(B3D_RENDER_BACKEND MATCHES "OpenGL")
	set(RENDER_API_MODULE_LIB bsfGLRenderAPI)
else()
	set(RENDER_API_MODULE_LIB bsfNullRenderAPI)
endif()

if(B3D_AUDIO_BACKEND MATCHES "FMOD")
	set(AUDIO_MODULE_LIB bsfFMOD)
elseif(B3D_AUDIO_BACKEND MATCHES "OpenAudio")
	set(AUDIO_MODULE_LIB bsfOpenAudio)
else()
	set(AUDIO_MODULE_LIB bsfNullAudio)
endif()

if(B3D_PHYSICS_BACKEND MATCHES "PhysX")
	set(PHYSICS_MODULE_LIB bsfPhysX)
else()
	set(PHYSICS_MODULE_LIB bsfNullPhysics)
endif()

if(B3D_RENDERER MATCHES "RenderBeast")
	set(RENDERER_MODULE_LIB bsfRenderBeast)
else()
	set(RENDERER_MODULE_LIB bsfNullRenderer)
endif()

if((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE)
	set(B3D_IS_SCRIPTING_ENABLED 1)
else()
	set(B3D_IS_SCRIPTING_ENABLED 0)
endif()

set(B3D_BUILD_TYPE_SHIPPING 0)
set(B3D_BUILD_TYPE_DEVELOPMENT 1)

if(B3D_BUILD_TYPE MATCHES "Shipping")
	set(B3D_BUILD_TYPE_ID B3D_BUILD_TYPE_SHIPPING)
else()
	set(B3D_BUILD_TYPE_ID B3D_BUILD_TYPE_DEVELOPMENT)
endif()

## Generate config files
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/BsEngineConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/bsfEngine/BsEngineConfig.h")
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/BsFrameworkConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/bsfUtility/BsFrameworkConfig.h")

# Set files disallowed in precompiled headers
set_directory_properties(PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Sub-directories
## Foundation
add_subdirectory(Foundation)

## Plugins
### If using MSVC include all plugins in the solution (which ones actually build is controlled by dependencies)
if(B3D_INCLUDE_ALL_IN_WORKFLOW AND MSVC)
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
	if(B3D_BUILD_ALL_RENDER_BACKENDS)
		if(WIN32)
			add_subdirectory(Plugins/bsfD3D11RenderAPI)
		endif()
		
		if(NOT APPLE)
			add_subdirectory(Plugins/bsfVulkanRenderAPI)
		endif()
		
		add_subdirectory(Plugins/bsfGLRenderAPI)
		add_subdirectory(Plugins/bsfNullRenderAPI)
	else()
		if(B3D_RENDER_BACKEND MATCHES "DirectX 11")
			add_subdirectory(Plugins/bsfD3D11RenderAPI)
		elseif(B3D_RENDER_BACKEND MATCHES "Vulkan")
			add_subdirectory(Plugins/bsfVulkanRenderAPI)
		elseif(B3D_RENDER_BACKEND MATCHES "OpenGL")
			add_subdirectory(Plugins/bsfGLRenderAPI)
		else()
			add_subdirectory(Plugins/bsfNullRenderAPI)
		endif()
	endif()

	if(B3D_AUDIO_BACKEND MATCHES "FMOD")
		add_subdirectory(Plugins/bsfFMOD)
	elseif(B3D_AUDIO_BACKEND MATCHES "OpenAudio")
		add_subdirectory(Plugins/bsfOpenAudio)
	else()
		add_subdirectory(Plugins/bsfNullAudio)
	endif()
	
	if(B3D_PHYSICS_BACKEND MATCHES "PhysX")
		add_subdirectory(Plugins/bsfPhysX)
	else()
		add_subdirectory(Plugins/bsfNullPhysics)
	endif()
	
	if(B3D_RENDERER MATCHES "RenderBeast")
		add_subdirectory(Plugins/bsfRenderBeast)
	else()
		add_subdirectory(Plugins/bsfNullRenderer)
	endif()
	
	if((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE)
		add_subdirectory(Scripting/bsfScript)
		add_subdirectory(Scripting/bsfSharpExec)
	endif()
	
	if((SCRIPT_API AND SCRIPT_API MATCHES "C#") OR B3D_IS_ENGINE)
		add_subdirectory(Scripting/bsfMono)
		add_subdirectory(Scripting/bsfSharp)
	endif()
endif()

add_subdirectory(Plugins/bsfFBXImporter)
add_subdirectory(Plugins/bsfFontImporter)
add_subdirectory(Plugins/bsfFreeImgImporter)
add_subdirectory(Plugins/bsfSL)

## Tests
if(B3D_BUILD_TESTS)
	enable_testing()

	add_executable(UtilityTest
		Tests/BsUtilityTest.cpp
		Tests/BsUtilityTestSuite.cpp
		Tests/BsFileSystemTestSuite.cpp
		Tests/BsECSTestSuite.cpp)
		
	target_link_libraries(UtilityTest bsf)
	target_include_directories(UtilityTest PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility"
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/ThirdParty"
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/Tests"
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/Tests/RTTI")

	add_executable(CoreTest
		Tests/BsCoreTest.cpp
		Tests/BsUnitTestComponents.h
		Tests/BsUnitTestComponents.cpp
		Tests/BsUnitTestSerializableObjects.h
		Tests/BsUnitTestSerializableObjects.cpp
		Tests/BsUnitTestSerializationHelper.h
		Tests/BsUnitTestSerializationHelper.cpp
		Tests/BsUnitTestPrefabUpdateHelper.h
		Tests/BsUnitTestPrefabUpdateHelper.cpp
		Tests/BsUnitTestScenes.h
		Tests/BsUnitTestScenes.cpp
		Tests/RTTI/BsUnitTestComponentsRTTI.h
		Tests/RTTI/BsUnitTestSerializableObjectsRTTI.h)
		
	target_link_libraries(CoreTest bsf)
	target_include_directories(CoreTest PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility"
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/ThirdParty"
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/Tests"
		"${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/Tests/RTTI")

	set_property(TARGET UtilityTest PROPERTY FOLDER Tests)
	set_property(TARGET CoreTest PROPERTY FOLDER Tests)	
	
	add_test(NAME UtilityTests COMMAND $<TARGET_FILE:UtilityTest>)
	add_test(NAME CoreTests COMMAND $<TARGET_FILE:UtilityTest>)
endif()

## Builtin resource preprocessing
add_executable(bsfImportTool
	Foundation/bsfEngine/Resources/BsBuiltinResourcesImporter.cpp)
B3DSetDefaultLinkAndCompileFlags(bsfImportTool)
target_compile_definitions(bsfImportTool PUBLIC -DBS_IS_ASSET_TOOL)

target_link_libraries(bsfImportTool bsf)
	
add_engine_dependencies(bsfImportTool)
add_importer_dependencies(bsfImportTool)

set_property(TARGET bsfImportTool PROPERTY FOLDER Tools)

if(NOT B3D_IS_ENGINE)
	install(TARGETS bsfImportTool RUNTIME DESTINATION bin)	
endif()
	
set(B3D_FTP_CREDENTIALS_FILE "${PROJECT_SOURCE_DIR}/../ftp_credentials" CACHE STRING "The location containing the FTP server credentials to use for uploading packages. The file is expected to contain three lines: URL/Username/Password, in that order.")
mark_as_advanced(B3D_FTP_CREDENTIALS_FILE)

if(${B3D_INCLUDE_ASSET_PACKAGING_SCRIPTS})
	B3DAddRunAssetImportTarget(bsfBuiltinAssets ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Data/Raw ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Data "")
	B3DAddUploadAssetsTarget(bsfBuiltinAssets ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Data Raw/DataPackageContents.txt)
	B3DAddUploadAssetsTarget(bsfDocImages ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Documentation DataPackageContents.txt)
endif()

set(B3D_FRAMEWORK_COMPILED_ASSETS_VERSION_FILE ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Data/.reqversion)
file (STRINGS ${B3D_FRAMEWORK_COMPILED_ASSETS_VERSION_FILE} B3D_FRAMEWORK_COMPILED_ASSETS_VERSION)
B3DUpdateBuiltinAssetsIfNeeded(bsfCompiled ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Data Data ${B3D_FRAMEWORK_COMPILED_ASSETS_VERSION})

set(B3D_DOCUMENTATION_ASSETS_VERSION_FILE ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Documentation/.reqversion)
file (STRINGS ${B3D_DOCUMENTATION_ASSETS_VERSION_FILE} B3D_DOCUMENTATION_ASSETS_VERSION)
B3DUpdateBuiltinAssetsIfNeeded(bsfDocImages ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Documentation Documentation ${B3D_DOCUMENTATION_ASSETS_VERSION})

if(NOT B3D_IS_ENGINE)
	set(B3D_ASSETS_INSTALL_FOLDER bin)
else()
	set(B3D_ASSETS_INSTALL_FOLDER .)
endif()

## Install assets
install(
	DIRECTORY ../Data
	DESTINATION ${B3D_ASSETS_INSTALL_FOLDER}
	PATTERN "../Data/Raw" EXCLUDE
	PATTERN ".version" EXCLUDE
	PATTERN ".reqversion" EXCLUDE
)

# TODO: These should be moved into their own folder for easier copy
install(
	DIRECTORY ../Data/Raw/Shaders
	DESTINATION ${B3D_ASSETS_INSTALL_FOLDER}/Data/Raw
)

install(
	FILES ../Data/Raw/GUI.css
	DESTINATION ${B3D_ASSETS_INSTALL_FOLDER}/Data/Raw
)
