# Version
set (B3D_FRAMEWORK_VERSION_MAJOR 2)
set (B3D_FRAMEWORK_VERSION_MINOR 0)
set (B3D_FRAMEWORK_VERSION_PATCH 0)
set (B3D_FRAMEWORK_CURRENT_RELEASE_YEAR 2025)

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

set(B3D_USE_DOTNETCORE ON CACHE BOOL "If enabled, uses new .NET Core 9.0 with support for C# 12. Otherwise uses .NET Framework 4.5 with support for C# 5.")

set(B3D_ENABLE_NETWORKING OFF CACHE BOOL "If true, enable networking support.")

set(B3D_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Use and install bundled libraries")

# Ensure dependencies are up to date
## Check data dependencies
B3DCheckAndUpdateAssetPackage(FrameworkData)
B3DCheckAndUpdateAssetPackage(FrameworkDataRaw)

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
	add_subdirectory(Tests)
endif()

## Builtin resource preprocessing
add_executable(BansheeImportTool
	Engine/Core/Resources/B3DBuiltinResourcesImporter.cpp)
B3DSetDefaultLinkAndCompileFlags(BansheeImportTool)

target_link_libraries(BansheeImportTool bsf)

B3DAddRuntimeDependencies(BansheeImportTool)
add_dependencies(BansheeImportTool bsfFBXImporter bsfFontImporter bsfFreeImgImporter bsfSL)

set_property(TARGET BansheeImportTool PROPERTY FOLDER Tools)

if(NOT B3D_IS_ENGINE)
	install(TARGETS BansheeImportTool RUNTIME DESTINATION bin)
endif()

include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/BuiltinAssetUtility.cmake)
B3DAddRunAssetImportTarget(FrameworkBuiltinAssets ${B3D_FRAMEWORK_ROOT_FOLDER}/Data/Raw ${B3D_FRAMEWORK_ROOT_FOLDER}/Data "")

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

install(
		DIRECTORY ../Data/Raw/UnitTestData
		DESTINATION ${B3D_ASSETS_INSTALL_FOLDER}/Data/Raw
)

install(
	DIRECTORY ../Data/Shaders
	DESTINATION ${B3D_ASSETS_INSTALL_FOLDER}/Data
)

install(
	FILES ../Data/GUI.css
	DESTINATION ${B3D_ASSETS_INSTALL_FOLDER}/Data
)
