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

# GPU backend choices and default are declared per-platform in Framework/Platform/<X>/Platform.cmake
# and collected during platform discovery (Prerequisites.cmake)
if(NOT B3D_GPU_BACKEND_CHOICES)
	message(FATAL_ERROR "No GPU backend was declared for the target platform. Ensure the target platform exists under Framework/Platform/ and its Platform.cmake declares a GPU backend.")
endif()

if(NOT B3D_GPU_BACKEND_DEFAULT)
	list(GET B3D_GPU_BACKEND_CHOICES 0 B3D_GPU_BACKEND_DEFAULT)
endif()

set(B3D_GPU_BACKEND "${B3D_GPU_BACKEND_DEFAULT}" CACHE STRING "GPU backend to use.")
set_property(CACHE B3D_GPU_BACKEND PROPERTY STRINGS ${B3D_GPU_BACKEND_CHOICES})

# The cached backend selection can go stale when B3D_PLATFORM changes in an existing build directory.
# Fall back to the platform's default instead of configuring with a backend the platform can't provide.
if(NOT B3D_GPU_BACKEND IN_LIST B3D_GPU_BACKEND_CHOICES)
	message(STATUS "GPU backend '${B3D_GPU_BACKEND}' is not available on platform '${B3D_PLATFORM}'. Falling back to '${B3D_GPU_BACKEND_DEFAULT}'.")
	set_property(CACHE B3D_GPU_BACKEND PROPERTY VALUE ${B3D_GPU_BACKEND_DEFAULT})
endif()

set(B3D_RENDERER "RenderBeast" CACHE STRING "Renderer backend to use.")
set_property(CACHE B3D_RENDERER PROPERTY STRINGS RenderBeast Null)

set(B3D_BUILD_EXAMPLES ON CACHE BOOL "If true, framework example projects will be built by default.")

set(B3D_BUILD_CODEGEN OFF CACHE BOOL "If true, code-generator project will be included as part of the main project.")

set(B3D_BUILD_ALL_PLUGINS OFF CACHE BOOL "If true, all plugins will be built regardless of backend selection. Also includes all plugins in the generated workflow.")

set(B3D_BUILD_TESTS OFF CACHE BOOL "If true, build targets for running unit tests will be included in the output.")

set(B3D_BUILD_BSL_PARSER OFF CACHE BOOL "If true, build lexer & parser for BSL. Requires flex & bison dependencies.")

set(B3D_BUILD_TYPE "Development" CACHE STRING "Determines the purpose of the build. 'Development' contains features to make debugging easier, but has overhead compared to other types - this should be your build type in most cases. 'Shipping' strips all development features.")
set_property(CACHE B3D_BUILD_TYPE PROPERTY STRINGS Development Shipping)

set(B3D_MONO_AOT OFF CACHE BOOL "If enabled, runs the Mono runtime in AOT+interpreter mode (MONO_AOT_MODE_INTERP, no JIT) and AOT-compiles managed assemblies at build time.")

set(B3D_ENABLE_NETWORKING OFF CACHE BOOL "If true, enable networking support.")

set(B3D_USE_BUNDLED_LIBRARIES ON CACHE BOOL "Use and install bundled libraries")

set(B3D_MONOLITHIC_BUILD OFF CACHE BOOL "Link non-importer plugins statically into a single bsf library.")

set(B3D_BUILD_IMPORTERS ON CACHE BOOL "Build asset importer plugins and the import tooling. These are cook-time only, and are not supported on consoles.")

set(B3D_BUILD_COOK_TOOL ON CACHE BOOL "Build the offline asset cook tool.")

# Apply platform-specific option overrides, if the target platform defines any.
if(COMMAND B3DApplyPlatformOptionOverrides)
	B3DApplyPlatformOptionOverrides()
endif()

# Ensure dependencies are up to date
## Check data dependencies
B3DCheckAndUpdateAssetPackage(FrameworkData)
B3DCheckAndUpdateAssetPackage(FrameworkDataRaw)

# Config file
## Note: Must happen before script binding generation
## Set names of libraries used in the config file
set(GPU_BACKEND_MODULE_LIB ${B3D_GPU_BACKEND_LIB_${B3D_GPU_BACKEND}})

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

## Active platform (uppercased name maps to the matching B3D_PLATFORM_ID_* constant)
string(TOUPPER ${B3D_PLATFORM} B3D_PLATFORM_UPPER)

## Generate config files
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/B3DEngineConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/Core/B3DEngineConfig.h")
configure_file("${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/B3DFrameworkConfig.h.in" "${PROJECT_BINARY_DIR}/Generated/Utility/B3DFrameworkConfig.h")

# Sub-directories
## Engine
add_subdirectory(Engine)

## Platform
### Each platform self-guards so only active platform fully contributes, but other platforms can still contribute if they need to (e.g. for tooling on a host platform).
foreach(platformName ${B3D_PLATFORM_CHOICES})
	set(platformSourceFolder ${B3D_PLATFORM_${platformName}_SOURCE_FOLDER})
	if(EXISTS ${platformSourceFolder}/CMakeLists.txt)
		add_subdirectory(${platformSourceFolder} ${PROJECT_BINARY_DIR}/Platform/${platformName})
	endif()
endforeach()

## Shader bytecode compilers
### Register these before bsfSL
if(B3D_BUILD_ALL_PLUGINS OR B3D_GPU_BACKEND STREQUAL "Vulkan" OR B3D_GPU_BACKEND STREQUAL "Metal" OR B3D_BUILD_COOK_TOOL)
	add_subdirectory(Plugins/ShaderBackends/bsfShaderBackendVKSL)
endif()

if(APPLE AND (B3D_BUILD_ALL_PLUGINS OR B3D_GPU_BACKEND STREQUAL "Vulkan" OR B3D_BUILD_COOK_TOOL))
	add_subdirectory(Plugins/ShaderBackends/bsfShaderBackendMVKSL)
endif()

if(APPLE AND (B3D_BUILD_ALL_PLUGINS OR B3D_GPU_BACKEND STREQUAL "Metal" OR B3D_BUILD_COOK_TOOL))
	add_subdirectory(Plugins/ShaderBackends/bsfShaderBackendMSL)
endif()

## Plugins
### If building all plugins, include all in the solution
if(B3D_BUILD_ALL_PLUGINS)
	add_subdirectory(Plugins/bsfVulkanGpuBackend)
	if(APPLE)
		add_subdirectory(Plugins/bsfMetalGpuBackend)
	endif()
	add_subdirectory(Plugins/bsfNullGpuBackend)
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
	# Match global GpuBackend plugins. Platforms can match their own.
	if(B3D_GPU_BACKEND STREQUAL "Vulkan")
		add_subdirectory(Plugins/bsfVulkanGpuBackend)
	elseif(B3D_GPU_BACKEND STREQUAL "Metal")
		add_subdirectory(Plugins/bsfMetalGpuBackend)
	elseif(B3D_GPU_BACKEND STREQUAL "Null")
		add_subdirectory(Plugins/bsfNullGpuBackend)
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

if(B3D_BUILD_IMPORTERS)
	add_subdirectory(Plugins/bsfFBXImporter)
	add_subdirectory(Plugins/bsfFontImporter)
	add_subdirectory(Plugins/bsfFreeImgImporter)
	add_subdirectory(Plugins/bsfSL)
endif()

# Offline asset cook tool
if(B3D_BUILD_COOK_TOOL)
	if(NOT TARGET bsfSL)
		add_subdirectory(Plugins/bsfSL)
	endif()

	if(NOT TARGET bsfNullGpuBackend)
		add_subdirectory(Plugins/bsfNullGpuBackend)
	endif()

	if(NOT TARGET bsfNullRenderer)
		add_subdirectory(Plugins/bsfNullRenderer)
	endif()

	if(NOT TARGET bsfRenderBeast)
		add_subdirectory(Plugins/bsfRenderBeast)
	endif()

	add_subdirectory(Tools/BansheeCookTool)
endif()

## Generate static plugin registry for monolithic builds
if(B3D_MONOLITHIC_BUILD)
	# B3D_STATIC_PLUGIN_TARGETS is accumulated by B3DAddPlugin calls
	set(B3D_STATIC_PLUGIN_DECLARATIONS "")
	set(B3D_STATIC_PLUGIN_ENTRIES "")
	set(B3D_STATIC_PLUGIN_FORCE_SYMBOLS "")

	foreach(PLUGIN_TARGET ${B3D_STATIC_PLUGIN_TARGETS})
		string(APPEND B3D_STATIC_PLUGIN_DECLARATIONS
			"extern \"C\" void* LoadPlugin_${PLUGIN_TARGET}();\nextern \"C\" void  UnloadPlugin_${PLUGIN_TARGET}(void*);\n"
		)
		string(APPEND B3D_STATIC_PLUGIN_ENTRIES
			"\t{ \"${PLUGIN_TARGET}\", LoadPlugin_${PLUGIN_TARGET}, UnloadPlugin_${PLUGIN_TARGET}, nullptr },\n"
		)

		if(MSVC)
			string(APPEND B3D_STATIC_PLUGIN_FORCE_SYMBOLS
				"#pragma comment(linker, \"/include:LoadPlugin_${PLUGIN_TARGET}\")\n"
			)
		else()
			string(APPEND B3D_STATIC_PLUGIN_FORCE_SYMBOLS
				"__attribute__((used)) static void* (*kForceKeep_${PLUGIN_TARGET})() = b3d::LoadPlugin_${PLUGIN_TARGET};\n"
			)
		endif()
	endforeach()

	# Wrap GCC/Clang force symbols in extern "C"
	if(NOT MSVC AND B3D_STATIC_PLUGIN_FORCE_SYMBOLS)
		set(B3D_STATIC_PLUGIN_FORCE_SYMBOLS
			"extern \"C\" {\n${B3D_STATIC_PLUGIN_FORCE_SYMBOLS}}\n"
		)
	endif()

	configure_file(
		${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/B3DStaticPluginRegistry.h.in
		${PROJECT_BINARY_DIR}/Generated/Core/B3DStaticPluginRegistry.h
	)

	target_link_libraries(bsf PRIVATE ${B3D_STATIC_PLUGIN_TARGETS})
endif()

## Tests
if(B3D_BUILD_TESTS)
	enable_testing()
	add_subdirectory(Tests)
endif()

## Builtin resource preprocessing
if(B3D_BUILD_IMPORTERS)
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
endif()

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
