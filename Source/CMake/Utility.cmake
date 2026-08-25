# Returns the list of platform directory names for the current target platform.
#
# @param	outFolderNames	Output list of active platform directory names.
function(B3DGetActivePlatformFolders outFolderNames)
	set(folders ${B3D_PLATFORM})

	# Unix sources are shared by the Linux and macOS targets.
	if(APPLE OR LINUX)
		list(APPEND folders Unix)
	endif()

	set(${outFolderNames} ${folders} PARENT_SCOPE)
endfunction()

# Determines library type based on monolithic mode and plugin category, and
# wires up the plugin's framework dependency.
# PLUGIN_CATEGORY: ENGINE (goes static in monolithic) or IMPORTER (always shared)
# bsf normally.
macro(B3DAddPlugin TARGET_NAME PLUGIN_CATEGORY)
	if(B3D_MONOLITHIC_BUILD AND NOT "${PLUGIN_CATEGORY}" STREQUAL "IMPORTER")
		add_library(${TARGET_NAME} STATIC ${ARGN})
		list(APPEND B3D_STATIC_PLUGIN_TARGETS ${TARGET_NAME})
		set(B3D_STATIC_PLUGIN_TARGETS ${B3D_STATIC_PLUGIN_TARGETS} PARENT_SCOPE)
		target_link_libraries(${TARGET_NAME} PRIVATE bsfHeaders)
		target_compile_definitions(${TARGET_NAME} PRIVATE B3D_EXPORTS)
	else()
		add_library(${TARGET_NAME} SHARED ${ARGN})
		target_link_libraries(${TARGET_NAME} PRIVATE bsf)
	endif()
endmacro()

# Registers a GPU backend implementation. The backend name must match the name the platform lists in
# B3D_GPU_BACKEND_CHOICES. 
#
# @param	backendName		Backend name as listed in B3D_GPU_BACKEND_CHOICES (e.g. Vulkan).
# @param	libraryTarget	The backend's plugin library target (e.g. bsfVulkanGpuBackend).
# @param	shadingLanguage	Identifier of the shading language the backend's prebuilt shaders are cooked in (e.g. vksl).
function(B3DRegisterGpuBackend backendName libraryTarget shadingLanguage)
	set_property(GLOBAL PROPERTY B3D_GPU_BACKEND_LIB_${backendName} ${libraryTarget})
	set_property(GLOBAL PROPERTY B3D_GPU_BACKEND_LANGUAGE_${backendName} ${shadingLanguage})
endfunction()

# Registers optional subdirectories based on selected properties.
function(B3DRegisterOptionalFrameworkSubdirectories)
	# Grab examples projects
	if(B3D_BUILD_EXAMPLES)
		find_path(EXAMPLE_SUBMODULE_SOURCES "CMakeLists.txt" PATHS "${B3D_FRAMEWORK_ROOT_FOLDER}/Examples" NO_DEFAULT_PATH NO_CACHE)
		if(NOT EXAMPLE_SUBMODULE_SOURCES)
			execute_process(COMMAND git submodule update
					--init
					-- Examples
					WORKING_DIRECTORY ${B3D_FRAMEWORK_ROOT_FOLDER})
		endif()
		mark_as_advanced(EXAMPLE_SUBMODULE_SOURCES)

		add_subdirectory(${B3D_FRAMEWORK_ROOT_FOLDER}/Examples)
	endif()

	# Grab code-generator project
	if(B3D_BUILD_CODEGEN)
		find_path(CODEGEN_SUBMODULE_SOURCES "CMakeLists.txt" PATHS "${B3D_TOOLS_DIRECTORY}/BansheeCodeGenerator" NO_DEFAULT_PATH NO_CACHE)
		if(NOT CODEGEN_SUBMODULE_SOURCES)
			execute_process(COMMAND git submodule update
					--init
					-- BansheeCodeGenerator
					WORKING_DIRECTORY ${B3D_TOOLS_DIRECTORY})
		endif()
		mark_as_advanced(CODEGEN_SUBMODULE_SOURCES)

		add_subdirectory(${B3D_TOOLS_DIRECTORY}/BansheeCodeGenerator)
		set_property(TARGET BansheeCodeGenerator PROPERTY FOLDER Tools)
	endif()

	# Script binding generation script
	if(((B3D_SCRIPT_API AND (NOT B3D_SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE) AND B3D_BUILD_CODEGEN)
		include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/GenerateScriptBindings.cmake)
	endif()

	# Documentation generation script
	if(B3D_BUILD_CODEGEN)
		include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/GenerateDocumentation.cmake)
	endif()

	# Stock icon generation script
	include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/StockIcons.cmake)
endfunction()

# Registers all relevant runtime dependencies for the specified target. This includes the minimal subset of
# dependencies for a standalone framework application (i.e. not the editor or tool).
#
# @param	target		Name of the target to add engine dependencies to.
function(B3DAddRuntimeDependencies target)
	if(${B3D_BUILD_ALL_PLUGINS})
		add_dependencies(${target} bsfVulkanGpuBackend bsfNullGpuBackend)
		if(TARGET bsfD3D12GpuBackend)
			add_dependencies(${target} bsfD3D12GpuBackend)
		endif()
		if(APPLE)
			add_dependencies(${target} bsfMetalGpuBackend)
		endif()
		add_dependencies(${target} bsfOpenAudio bsfNullAudio)
		add_dependencies(${target} bsfPhysX bsfNullPhysics)
		add_dependencies(${target} bsfRenderBeast bsfNullRenderer)
	else()
		get_property(gpuBackendLibrary GLOBAL PROPERTY B3D_GPU_BACKEND_LIB_${B3D_GPU_BACKEND})
		add_dependencies(${target} ${gpuBackendLibrary})

		if(B3D_AUDIO_BACKEND MATCHES "FMOD")
			add_dependencies(${target} bsfFMOD)
		elseif(B3D_AUDIO_BACKEND MATCHES "OpenAudio")
			add_dependencies(${target} bsfOpenAudio)
		else()
			add_dependencies(${target} bsfNullAudio)
		endif()

		if(B3D_PHYSICS_BACKEND MATCHES "PhysX")
			add_dependencies(${target} bsfPhysX)
		else()
			add_dependencies(${target} bsfNullPhysics)
		endif()

		if(B3D_RENDERER MATCHES "RenderBeast")
			add_dependencies(${target} bsfRenderBeast)
		else()
			add_dependencies(${target} bsfNullRenderer)
		endif()
	endif()

	if(B3D_BUILD_IMPORTERS)
		add_dependencies(${target} bsfSL)
	endif()
endfunction()

# Links the provided framework to the provided target (Apple only).
#
# @param	target		Target to link the framework to.
# param		framework	Framework to link.
function(B3DTargetLinkFramework target framework)
	find_library(FM_${framework} ${framework})

	if(NOT FM_${framework})
		message(FATAL_ERROR "Cannot find ${framework} framework.")
	endif()

	target_link_libraries(${target} PRIVATE ${FM_${framework}})
	mark_as_advanced(FM_${framework})
endfunction()

# Strips symbols that are embedded in the target executable, and saves them in a separate file.
#
# @param	targetName		Name of the target from whose executable or library to strip the symbols.
# @param	outputFilename	Filename of the file containing the stripped symbols.
function(B3DStripSymbols targetName outputFilename)
	if(UNIX AND B3D_STRIP_DEBUG_INFO)
		if(CMAKE_BUILD_TYPE STREQUAL Release)
			set(fileToStrip $<TARGET_FILE:${targetName}>)

			# macOS
			if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
				set(symbolsFile ${fileToStrip}.dwarf)

				add_custom_command(
					TARGET ${targetName}
					POST_BUILD
					VERBATIM
					COMMAND ${DSYMUTIL_TOOL} --flat --minimize ${fileToStrip}
					COMMAND ${STRIP_TOOL} -u -r ${fileToStrip}
					COMMENT Stripping symbols from ${fileToStrip} into file ${symbolsFile}
				)
			
			# Linux
			else()
				set(symbolsFile ${fileToStrip}.dbg)

				add_custom_command(
					TARGET ${targetName}
					POST_BUILD
					VERBATIM
					COMMAND ${OBJCOPY_TOOL} --only-keep-debug ${fileToStrip} ${symbolsFile}
					COMMAND ${OBJCOPY_TOOL} --strip-unneeded ${fileToStrip}
					COMMAND ${OBJCOPY_TOOL} --add-gnu-debuglink=${symbolsFile} ${fileToStrip}
					COMMENT Stripping symbols from ${fileToStrip} into file ${symbolsFile}
				)
			endif(CMAKE_SYSTEM_NAME STREQUAL Darwin)

			set(${outputFilename} ${symbolsFile} PARENT_SCOPE)
		endif()
	endif()
endfunction()
