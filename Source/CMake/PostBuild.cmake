# Post-build and install step helpers for engine targets.

# Sets up required post-build and install steps for the provided target. This should be called on all non-imported
# targets. This should be called after all the relevant libraries have been linked to the target.
#
# In particular these are the operations performed:
# 1. Sets up a post-build step that strips symbols embedded in the executable.
# 2. Sets up a post-build step that copies any linked imported binaries (e.g. dll) into the build directory
# 3. Sets up an install step that installs all output artifacts from the provided target
# 4. Sets up an install step that installs the symbol file for the target
# 5. Sets up an install step that installs any linked imported binaries (e.g. dll)
function(B3DSetUpPostBuildAndInstallSteps target)

	# Strip symbols
	B3DStripSymbols(${target} symbolsFile)

	# Set up directory in which to output the binaries
	if(NOT B3D_IS_ENGINE)
		set(installBinaryDirectory bin)
	else()
		set(installBinaryDirectory .)
	endif()

	get_target_property(targetType ${target} TYPE)
	if (NOT targetType STREQUAL "STATIC_LIBRARY")
		# Install target artifacts
		install(
				TARGETS ${target}
				EXPORT bsf
				RUNTIME DESTINATION ${installBinaryDirectory}
		)

		# Install symbol file
		if(MSVC)
			install(
					FILES $<TARGET_PDB_FILE:${target}>
					DESTINATION ${installBinaryDirectory}
					OPTIONAL
			)
		else()
			install(
					FILES ${symbolsFile}
					DESTINATION lib
					OPTIONAL)
		endif()
	endif()

	# Find all linked imported libraries
	get_target_property(libraries ${target} LINK_LIBRARIES)
	foreach(library ${libraries})
		if(NOT TARGET ${library})
			continue()
		endif()

		get_target_property(isImported ${library} IMPORTED)
		if(NOT ${isImported})
			continue()
		endif()

		get_target_property(libraryImportLocation ${library} IMPORTED_LOCATION)

		cmake_path(IS_PREFIX PROJECT_SOURCE_DIR ${libraryImportLocation} isInProjectFolder)
		get_target_property(deployImportedRuntime ${library} B3D_DEPLOY_IMPORTED_RUNTIME)
		if(NOT ${isInProjectFolder} AND NOT deployImportedRuntime)
			continue()
		endif()

		get_filename_component(libraryFileName ${libraryImportLocation} NAME)

		# TODO: Not handling Linux/macOS
		if(NOT libraryFileName MATCHES "\.dll$")
			continue()
		endif()

		get_target_property(libraryImportLocationDebug ${library} IMPORTED_LOCATION_DEBUG)

		set(source ${libraryImportLocation})
		set(debugSource ${libraryImportLocationDebug})
		set(destination $<TARGET_FILE_DIR:bsf>)

		# Copy imported library on build
		add_custom_command(
				TARGET ${target} POST_BUILD
				COMMAND ${CMAKE_COMMAND}
				ARGS    -E copy_if_different $<$<CONFIG:Debug>:${debugSource}>$<$<NOT:$<CONFIG:Debug>>:${source}> ${destination}
				COMMENT "Copying $<$<CONFIG:Debug>:${debugSource}>$<$<NOT:$<CONFIG:Debug>>:${source}> to ${destination}\n"
				COMMAND_EXPAND_LISTS
		)

		# Install imported library
		install(
				FILES ${source}
				DESTINATION ${installBinaryDirectory}
				CONFIGURATIONS Release RelWithDebInfo MinSizeRel
		)

		install(
				FILES ${debugSource}
				DESTINATION ${installBinaryDirectory}
				CONFIGURATIONS Debug
		)
	endforeach()
endfunction()

# Copies the provided folder when @p target is built.
#
# @param	target					Target which needs to build to trigger the copy operation.
# @param	sourceParentFolder		Location containing the folder to copy from.
# @param	destinationParentFolder	Location to copy the folder to.
# @param	folderName				Name of the folder to copy.
# @param	filter					Filter to copy only certain files, *.* to copy all files.
function(B3DCopyFolderOnBuild target sourceParentFolder destinationParentFolder folderName filter)
	set(sourceFolder ${sourceParentFolder}/${folderName})
	set(destinationFolder ${destinationParentFolder}/${folderName})
	
	file(GLOB_RECURSE allFiles RELATIVE ${sourceFolder} "${sourceFolder}/${filter}")

	foreach(currentFilePath ${allFiles})
		get_filename_component(FILENAME ${currentFilePath} NAME)
	
		set(sourceFilePath ${sourceFolder}/${currentFilePath})
		set(destinationFilePath ${destinationFolder}/${currentFilePath})
		add_custom_command(
		   TARGET ${target} POST_BUILD
		   COMMAND ${CMAKE_COMMAND}
		   ARGS    -E copy_if_different ${sourceFilePath} ${destinationFilePath}
		   COMMENT "Copying ${sourceFilePath} ${destinationFilePath}"
		   )
	endforeach()
endfunction()
