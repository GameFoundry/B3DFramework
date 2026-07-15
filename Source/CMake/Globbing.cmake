	# Parses all source files in the provided folder recursively, and outputs the list of files in @p outSourceFiles.
# For each sub-folder found it also creates a source group with the folder hierarchy.
#
# @param	parentPath		Folder path in which to perform the search. All output paths will be relative to this path.
# @param	pattern			Extensions of the source files to search for, e.g. "cpp|c|hpp|h"
# @param	path			Additional path to append after @p parentPath. This will limit the search to this particular
#							sub-path, but the output paths will still remain relative to @p parentPath. Must include
#							the closing '/' if not empty.
# @param	foldersToIgnore	Optional list of folders to ignore in the search
# @param	outSourceFiles	List of all found source files, relative to @p parentPath.
function(B3DGlobSourceFilesWithExplicitPattern parentPath pattern path foldersToIgnore outSourceFiles)
	if(NOT path)
		# Need to assign some value to path, otherwise the list below is treated as empty
		set(path "/")
	endif()

	list(APPEND pathsToProcess ${path})
	list(APPEND sourceGroups "")

	set(sourceFiles "")
	while(pathsToProcess)
		list(POP_FRONT pathsToProcess currentPath)

		if(${currentPath} STREQUAL "/")
			set(currentPath "")
		endif()

		set(fullPath "${parentPath}/${currentPath}")
		file(GLOB directChildren RELATIVE "${fullPath}" "${fullPath}*")

		set(sourceGroupFiles "")
		list(POP_FRONT sourceGroups currentSourceGroup)

		# For each direct child (file or folder) of the current path
		foreach(child ${directChildren})
			# If folder, check all the files in the folder, and define a source group for it. Any child folders
			if(IS_DIRECTORY "${fullPath}${child}")

				# Skip platform specific files that aren't for the current platform
				if( (${child} MATCHES "Win32" AND NOT WIN32) OR
				(${child} MATCHES "MacOS" AND NOT APPLE) OR
				(${child} MATCHES "macOS" AND NOT APPLE) OR
				(${child} MATCHES "Linux" AND NOT LINUX) OR
				(${child} MATCHES "Unix" AND NOT LINUX AND NOT APPLE))
					continue()
				endif()

				if(${child} IN_LIST foldersToIgnore)
					continue()
				endif()

				list(APPEND pathsToProcess ${currentPath}${child}/)
				list(APPEND sourceGroups ${currentSourceGroup}${child}/)
			else()
				# [.] is a literal-dot anchor (CMake regex collapses \. to any-char); () makes the list an alternation.
				if(${child} MATCHES ".*[.](${pattern})$")
					list(APPEND sourceFiles "${currentPath}${child}")
					list(APPEND sourceGroupFiles "${currentPath}${child}")
				endif()
			endif()

			if(sourceGroupFiles)
				if(NOT currentSourceGroup)
					source_group("" FILES ${sourceGroupFiles})
				else()
					source_group(${currentSourceGroup} FILES ${sourceGroupFiles})
				endif()
			endif()

		endforeach()

	endwhile()

	list(APPEND sourceFiles ${${outSourceFiles}})
	set(${outSourceFiles} ${sourceFiles} PARENT_SCOPE)
endfunction()

# Parses all C++ source files in the provided folder recursively, and outputs the list of files in @p outSourceFiles.
# For each sub-folder found it also creates a source group with the folder hierarchy.
#
# @param	parentPath		Folder path in which to perform the search. All output paths will be relative to this path.
# @param	path			Additional path to append after @p parentPath. This will limit the search to this particular
#							sub-path, but the output paths will still remain relative to @p parentPath. Must include
#							the closing '/' if not empty.
# @param	foldersToIgnore	Optional list of folders to ignore in the search
# @param	outSourceFiles	List of all found source files, relative to @p parentPath.
function(B3DGlobSourceFiles parentPath path foldersToIgnore outSourceFiles)
	# .S assembly files only compile on platforms that enable the ASM language (Platform.cmake); elsewhere they are carried as inert project items.
	B3DGlobSourceFilesWithExplicitPattern("${parentPath}" "cpp|cxx|cc|c|hpp|hh|h|inl|mm|m|rc|S" "${path}" "${foldersToIgnore}" sourceFiles)

	list(APPEND sourceFiles ${${outSourceFiles}})
	set(${outSourceFiles} ${sourceFiles} PARENT_SCOPE)
endfunction()

# Globs source files from the Source/Engine path and the active platform Platform/{Platform}/Source/Engine path.
#
# Non-platform file paths are returned relative to Framework/Source/Engine,
# platform overlay file paths are returned absolute (they live outside that tree).
#
# @param	enginePath		Engine sub-folder to collect, e.g. "Core/" or "Utility/".
# @param	foldersToIgnore	Folders to ignore (applied to both trees).
# @param	outSourceFiles	Output list of collected source files.
function(B3DGlobSourceFilesAcrossPlatforms enginePath foldersToIgnore outSourceFiles)
	set(collected "")

	# 1) Framework engine sources
	B3DGlobSourceFiles(${B3D_FRAMEWORK_SOURCE_FOLDER}/Engine "${enginePath}" "${foldersToIgnore}" collected)

	# 2) Active platform engine sources
	string(REGEX REPLACE "/$" "" engineSub "${enginePath}")
	B3DGetActivePlatformFolders(activePlatforms)
	foreach(platformName ${activePlatforms})
		set(platformEngineFolder ${B3D_PLATFORM_${platformName}_SOURCE_FOLDER}/Engine)
		if(NOT EXISTS ${platformEngineFolder}/${engineSub})
			continue()
		endif()

		set(platformFiles "")
		B3DGlobSourceFiles(${platformEngineFolder} "${enginePath}" "${foldersToIgnore}" platformFiles)

		# Convert to absolute paths (these live outside the Engine tree) and group per platform.
		foreach(relFile ${platformFiles})
			set(absFile ${platformEngineFolder}/${relFile})
			list(APPEND collected ${absFile})

			get_filename_component(relDir ${relFile} DIRECTORY)
			source_group(${platformName}/${relDir} FILES ${absFile})
		endforeach()
	endforeach()

	set(${outSourceFiles} ${collected} PARENT_SCOPE)
endfunction()
