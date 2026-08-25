# Offline shader cooking. Input folders containing raw shader code are register by appending to the
# B3D_SHADER_COOK_INPUT_FOLDERS global property before B3DAddShaderCookTarget() is called.

# Registers the B3DCookShaders target that cooks the prebuilt shader store. Call once, after every shader folder has been registered.
function(B3DAddShaderCookTarget)
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
		set(cookToolDependency BansheeCookTool)
	else()
		set(B3D_HOST_TOOLS_DIR "" CACHE PATH "Binaries folder of a host build (containing BansheeCookTool and its libraries), for build trees that cannot build the host tools themselves.")

		if(CMAKE_HOST_WIN32)
			set(cookTool "${B3D_HOST_TOOLS_DIR}/BansheeCookTool.exe")
		else()
			set(cookTool "${B3D_HOST_TOOLS_DIR}/BansheeCookTool")
		endif()

		if(NOT B3D_HOST_TOOLS_DIR OR NOT EXISTS "${cookTool}")
			message(WARNING "BansheeCookTool was not found (set B3D_HOST_TOOLS_DIR to a host build's binaries folder); the prebuilt shader store will not be cooked.")
			return()
		endif()

		set(cookToolDependency "")
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

	# The tool takes the folders as a single ';'-separated parameter; $<SEMICOLON> keeps CMake from splitting it.
	list(JOIN cookInputFolders "$<SEMICOLON>" cookInputParameter)

	add_custom_command(
		OUTPUT "${storePath}"
		COMMAND "${cookTool}" -input "${cookInputParameter}" -output "${storePath}" -language ${cookLanguage} --debug.DisableErrorDialogs=true
		DEPENDS ${cookInputFiles} ${cookToolDependency}
		COMMENT "Cooking prebuilt shader store (${cookLanguage})"
		VERBATIM)

	add_custom_target(B3DCookShaders ALL DEPENDS "${storePath}")
	set_property(TARGET B3DCookShaders PROPERTY FOLDER Tools)
endfunction()
