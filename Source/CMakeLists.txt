# Version
set (B3D_FRAMEWORK_VERSION_MAJOR 2)
set (B3D_FRAMEWORK_VERSION_MINOR 0)
set (B3D_FRAMEWORK_VERSION_PATCH 0)
set (B3D_FRAMEWORK_CURRENT_RELEASE_YEAR 2025)

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
	set_property(CACHE B3D_RENDER_BACKEND PROPERTY STRINGS "Vulkan" "Null")
elseif(APPLE)
	set(B3D_RENDER_BACKEND "Vulkan" CACHE STRING "Render API to use.")
	set_property(CACHE B3D_RENDER_BACKEND PROPERTY STRINGS "Vulkan" "Null")
else()
	set(B3D_RENDER_BACKEND "Vulkan" CACHE STRING "Render API to use.")
	set_property(CACHE B3D_RENDER_BACKEND PROPERTY STRINGS "Vulkan" "Null")
endif()

set(B3D_RENDERER "RenderBeast" CACHE STRING "Renderer backend to use.")
set_property(CACHE B3D_RENDERER PROPERTY STRINGS RenderBeast Null)

set(B3D_BUILD_EXAMPLES ON CACHE BOOL "If true, framework example projects will be built by default.")

set(B3D_BUILD_CODEGEN OFF CACHE BOOL "If true, code-generator project will be included as part of the main project.")

set(B3D_INCLUDE_ALL_IN_WORKFLOW OFF CACHE BOOL "If true, all libraries (even those not selected) will be included in the generated workflow (e.g. Visual Studio solution). This is useful when working on engine internals with a need for easy access to all parts of it. Only relevant for workflow generators like Visual Studio or XCode.")

set(B3D_BUILD_TESTS OFF CACHE BOOL "If true, build targets for running unit tests will be included in the output.")

set(B3D_BUILD_BSL_PARSER OFF CACHE BOOL "If true, build lexer & parser for BSL. Requires flex & bison dependencies.")

set(B3D_BUILD_ALL_RENDER_BACKENDS OFF CACHE BOOL "If true, all supported render backends will be built, regardless of choice in B3D_RENDER_BACKEND. Choice in B3D_RENDER_BACKEND will still be used as the default.")

set(B3D_BUILD_TYPE "Development" CACHE STRING "Determines the purpose of the build. 'Development' contains features to make debugging easier, but has overhead compared to other types - this should be your build type in most cases. 'Shipping' strips all development features.")
set_property(CACHE B3D_BUILD_TYPE PROPERTY STRINGS Development Shipping)

set(B3D_INCLUDE_ASSET_PACKAGING_SCRIPTS OFF CACHE BOOL "If true, generate helper targets that allow you to import built-in assets as well as upload new package versions.")

set(B3D_UPLOAD_ASSETS OFF CACHE BOOL "Only relevant if BUILD_ASSETS is true. When enabled the system will upload a package with the latest assets whenever assets are built, provided valid server and credentials are present.")

set(B3D_USE_DOTNETCORE ON CACHE BOOL "If enabled, uses new .NET Core 9.0 with support for C# 12. Otherwise uses .NET Framework 4.5 with support for C# 5.")

set(B3D_ENABLE_NETWORKING OFF CACHE BOOL "If true, enable networking support.")

set(B3D_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Use and install bundled libraries")

# Ensure dependencies are up to date
## Check prebuilt dependencies that are downloaded in a .zip
B3DCheckAndUpdatePrebuiltDependencies(bsfDependencies ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/ Dependencies ${B3D_FRAMEWORK_PREBUILT_DEPENDENCIES_VERSION})

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
if(B3D_RENDER_BACKEND MATCHES "Vulkan")
	set(RENDER_API_MODULE_LIB bsfVulkanRenderAPI)
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

if((B3D_SCRIPT_API AND (NOT B3D_SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE)
	set(B3D_IS_SCRIPTING_ENABLED 1)
else()
	set(B3D_IS_SCRIPTING_ENABLED 0)
endif()

set(B3D_BUILD_TYPE_SHIPPING_ID 0)
set(B3D_BUILD_TYPE_DEVELOPMENT_ID 1)
set(B3D_BUILD_TYPE_PROFILING_ID 2)

if(B3D_BUILD_TYPE MATCHES "Shipping")
	set(B3D_BUILD_TYPE_ID B3D_BUILD_TYPE_SHIPPING_ID)
else()
	set(B3D_BUILD_TYPE_ID B3D_BUILD_TYPE_DEVELOPMENT_ID)
endif()

## Testing configuration
set(B3D_ENABLE_TESTS ${B3D_BUILD_TESTS})

## Generate config files
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/B3DEngineConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/Core/B3DEngineConfig.h")
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/B3DFrameworkConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/Utility/B3DFrameworkConfig.h")

# Sub-directories
## Engine
add_subdirectory(Engine)

## Plugins
### If using MSVC include all plugins in the solution (which ones actually build is controlled by dependencies)
if(B3D_INCLUDE_ALL_IN_WORKFLOW AND MSVC)
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
		add_subdirectory(Plugins/bsfVulkanRenderAPI)
		add_subdirectory(Plugins/bsfNullRenderAPI)
	else()
		if(B3D_RENDER_BACKEND MATCHES "Vulkan")
			add_subdirectory(Plugins/bsfVulkanRenderAPI)
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
	
	if((B3D_SCRIPT_API AND (NOT B3D_SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE)
		add_subdirectory(Scripting/bsfScript)
		add_subdirectory(Scripting/bsfSharpExec)
	endif()
	
	if((B3D_SCRIPT_API AND B3D_SCRIPT_API MATCHES "C#") OR B3D_IS_ENGINE)
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
		Tests/B3DUtilityTest.cpp
		Tests/B3DUtilityTestSuite.cpp
		Tests/B3DFileSystemTestSuite.cpp
		Tests/B3DECSTestSuite.cpp)
		
	target_link_libraries(UtilityTest bsf)
	target_include_directories(UtilityTest PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility"
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility/ThirdParty"
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility/Tests"
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility/Tests/RTTI")

	add_executable(CoreTest
		Tests/B3DCoreTest.cpp
		Tests/B3DUnitTestComponents.h
		Tests/B3DUnitTestComponents.cpp
		Tests/B3DUnitTestSerializableObjects.h
		Tests/B3DUnitTestSerializableObjects.cpp
		Tests/B3DUnitTestSerializationHelper.h
		Tests/B3DUnitTestSerializationHelper.cpp
		Tests/B3DUnitTestPrefabUpdateHelper.h
		Tests/B3DUnitTestPrefabUpdateHelper.cpp
		Tests/B3DUnitTestScenes.h
		Tests/B3DUnitTestScenes.cpp
		Tests/RTTI/B3DUnitTestComponentsRTTI.h
		Tests/RTTI/B3DUnitTestSerializableObjectsRTTI.h)
		
	target_link_libraries(CoreTest bsf)
	target_include_directories(CoreTest PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility"
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility/ThirdParty"
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility/Tests"
		"${CMAKE_CURRENT_SOURCE_DIR}/Utility/Tests/RTTI")

	set_property(TARGET UtilityTest PROPERTY FOLDER Tests)
	set_property(TARGET CoreTest PROPERTY FOLDER Tests)	
	
	add_test(NAME UtilityTests COMMAND $<TARGET_FILE:UtilityTest>)
	add_test(NAME CoreTests COMMAND $<TARGET_FILE:UtilityTest>)
endif()

## Builtin resource preprocessing
add_executable(bsfImportTool
	Engine/Core/Resources/B3DBuiltinResourcesImporter.cpp)
B3DSetDefaultLinkAndCompileFlags(bsfImportTool)
target_compile_definitions(bsfImportTool PUBLIC -DBS_IS_ASSET_TOOL)

target_link_libraries(bsfImportTool bsf)
	
B3DAddRuntimeDependencies(bsfImportTool)
add_dependencies(bsfImportTool bsfFBXImporter bsfFontImporter bsfFreeImgImporter bsfSL)

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
