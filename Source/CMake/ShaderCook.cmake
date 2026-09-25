# Offline shader cooking. Input folders containing raw shader code are registered by appending to the
# B3D_SHADER_COOK_INPUT_FOLDERS global property, and libraries the cook must load are registered through
# B3DRegisterShaderCookModule(). Both must happen before B3DAddShaderCookTarget() runs.

# Registers a library the shader cook must load, because loading it registers content the cook needs (currently
# renderer materials declared with RMAT_DEF).
#
# @param	target	Name of the library target. Must be a shared library, or a plugin statically linked into the framework.
function(B3DRegisterShaderCookModule target)
	set_property(GLOBAL APPEND PROPERTY B3D_SHADER_COOK_MODULES ${target})
endfunction()

# Outputs the libraries registered through B3DRegisterShaderCookModule(), after checking that the cook tool can load
# each of them.
#
# @param	outModules	List of module target names.
function(B3DGetShaderCookModules outModules)
	get_property(modules GLOBAL PROPERTY B3D_SHADER_COOK_MODULES)
	list(REMOVE_DUPLICATES modules)

	get_target_property(bsfLinkLibraries bsf LINK_LIBRARIES)
	foreach(module ${modules})
		if(NOT TARGET ${module})
			message(FATAL_ERROR "Shader cook module '${module}' is not a target.")
		endif()

		get_target_property(outputName ${module} OUTPUT_NAME)
		if(outputName)
			message(FATAL_ERROR "Shader cook module '${module}' sets OUTPUT_NAME, but the cook tool loads modules by target name.")
		endif()

		get_target_property(moduleType ${module} TYPE)
		if(moduleType STREQUAL "STATIC_LIBRARY")
			if(NOT module IN_LIST bsfLinkLibraries)
				message(FATAL_ERROR "Shader cook module '${module}' is a static library that is not linked into bsf, so the cook tool cannot load it.")
			endif()
		elseif(NOT moduleType STREQUAL "SHARED_LIBRARY" AND NOT moduleType STREQUAL "MODULE_LIBRARY")
			message(FATAL_ERROR "Shader cook module '${module}' is a ${moduleType} target, but the cook tool can only load libraries.")
		endif()
	endforeach()

	set(${outModules} ${modules} PARENT_SCOPE)
endfunction()

# Registers the B3DCookShaders target that cooks the prebuilt shader store. Call once, after every shader folder and
# shader cook module has been registered.
function(B3DAddShaderCookTarget)
	B3DGetShaderCookModules(cookModules)

	if(TARGET BansheeCookTool AND cookModules)
		add_dependencies(BansheeCookTool ${cookModules})
	endif()

	get_property(cookLanguage GLOBAL PROPERTY B3D_GPU_BACKEND_LANGUAGE_${B3D_GPU_BACKEND})
	if(NOT cookLanguage)
		message(WARNING "GPU backend '${B3D_GPU_BACKEND}' did not register a shading language (B3DRegisterGpuBackend); the prebuilt shader store will not be cooked.")
		return()
	endif()

	get_property(cookInputFolders GLOBAL PROPERTY B3D_SHADER_COOK_INPUT_FOLDERS)
	if(NOT cookInputFolders)
		return()
	endif()

	if(TARGET BansheeCookTool)
		set(cookTool "$<TARGET_FILE:BansheeCookTool>")
		set(cookToolDependency BansheeCookTool ${cookModules})
	else()
		set(B3D_HOST_TOOLS_DIR "" CACHE PATH "Binaries folder of a host build (containing BansheeCookTool and its libraries), for build trees that cannot build the host tools themselves.")

		if(CMAKE_HOST_WIN32)
			set(cookTool "${B3D_HOST_TOOLS_DIR}/BansheeCookTool.exe")
			set(hostLibraryPrefix "")
			set(hostLibrarySuffix ".dll")
		elseif(CMAKE_HOST_APPLE)
			set(cookTool "${B3D_HOST_TOOLS_DIR}/BansheeCookTool")
			set(hostLibraryPrefix "lib")
			set(hostLibrarySuffix ".dylib")
		else()
			set(cookTool "${B3D_HOST_TOOLS_DIR}/BansheeCookTool")
			set(hostLibraryPrefix "lib")
			set(hostLibrarySuffix ".so")
		endif()

		if(NOT B3D_HOST_TOOLS_DIR OR NOT EXISTS "${cookTool}")
			message(WARNING "BansheeCookTool was not found (set B3D_HOST_TOOLS_DIR to a host build's binaries folder); the prebuilt shader store will not be cooked.")
			return()
		endif()

		# A module missing from the host folder is statically linked into the host's bsf, or was not built there. The
		# cook tool reports the latter when it runs.
		set(cookToolDependency "${cookTool}")
		foreach(module ${cookModules})
			set(hostLibrary "${B3D_HOST_TOOLS_DIR}/${hostLibraryPrefix}${module}${hostLibrarySuffix}")
			if(EXISTS "${hostLibrary}")
				list(APPEND cookToolDependency "${hostLibrary}")
			endif()
		endforeach()
	endif()

	# Store location: the tree's runtime output folder for the built configuration.
	get_filename_component(runtimeOutputRoot "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}" DIRECTORY)
	set(storePath "${runtimeOutputRoot}/$<CONFIG>/CompiledShaders/Shaders.b3d")

	# Re-cook when any shader source changes. CONFIGURE_DEPENDS re-runs the glob when files are added or removed.
	set(globPatterns "")
	foreach(cookInputFolder ${cookInputFolders})
		list(APPEND globPatterns "${cookInputFolder}/*.bsl" "${cookInputFolder}/*.bslinc")
	endforeach()
	file(GLOB_RECURSE cookInputFiles CONFIGURE_DEPENDS ${globPatterns})

	# The tool takes folders and modules as single ';'-separated parameters; $<SEMICOLON> keeps CMake from splitting them.
	list(JOIN cookInputFolders "$<SEMICOLON>" cookInputParameter)

	set(cookModuleArguments "")
	if(cookModules)
		list(JOIN cookModules "$<SEMICOLON>" cookModuleParameter)
		set(cookModuleArguments -modules "${cookModuleParameter}")
	endif()

	add_custom_command(
		OUTPUT "${storePath}"
		# CMake also tracks library changes, which the tool's shader-source timestamp check cannot detect.
		COMMAND "${cookTool}" -force -input "${cookInputParameter}" ${cookModuleArguments} -output "${storePath}" -language ${cookLanguage} --debug.DisableErrorDialogs=true
		DEPENDS ${cookInputFiles} ${cookToolDependency}
		COMMENT "Cooking prebuilt shader store (${cookLanguage})"
		VERBATIM)

	add_custom_target(B3DCookShaders ALL DEPENDS "${storePath}")
	set_property(TARGET B3DCookShaders PROPERTY FOLDER Tools)
endfunction()
