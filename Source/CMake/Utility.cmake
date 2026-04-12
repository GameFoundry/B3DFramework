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
				if(${child} MATCHES ".*\.[${pattern}]$")
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
	B3DGlobSourceFilesWithExplicitPattern("${parentPath}" "cpp|cxx|c|hpp|h|inl|mm|m" "${path}" "${foldersToIgnore}" sourceFiles)

	list(APPEND sourceFiles ${${outSourceFiles}})
	set(${outSourceFiles} ${sourceFiles} PARENT_SCOPE)
endfunction()

# Determines library type based on monolithic mode and plugin category.
# PLUGIN_CATEGORY: ENGINE (goes static in monolithic) or IMPORTER (always shared)
macro(B3DAddPlugin TARGET_NAME PLUGIN_CATEGORY)
	if(B3D_MONOLITHIC_BUILD AND NOT "${PLUGIN_CATEGORY}" STREQUAL "IMPORTER")
		add_library(${TARGET_NAME} STATIC ${ARGN})
		list(APPEND B3D_STATIC_PLUGIN_TARGETS ${TARGET_NAME})
		set(B3D_STATIC_PLUGIN_TARGETS ${B3D_STATIC_PLUGIN_TARGETS} PARENT_SCOPE)
	else()
		add_library(${TARGET_NAME} SHARED ${ARGN})
	endif()
endmacro()

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

	# Stock icon generation script
	include(${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/StockIcons.cmake)
endfunction()

# Registers all relevant runtime dependencies for the specified target. This includes the minimal subset of
# dependencies for a standalone framework application (i.e. not the editor or tool).
#
# @param	target		Name of the target to add engine dependencies to.
function(B3DAddRuntimeDependencies target)
	if(${B3D_BUILD_ALL_PLUGINS})
		add_dependencies(${target} bsfVulkanRenderAPI bsfNullRenderAPI)
		add_dependencies(${target} bsfOpenAudio bsfNullAudio)
		add_dependencies(${target} bsfPhysX bsfNullPhysics)
		add_dependencies(${target} bsfRenderBeast bsfNullRenderer)
	else()
		if(B3D_RENDER_BACKEND MATCHES "Vulkan")
			add_dependencies(${target} bsfVulkanRenderAPI)
		else()
			add_dependencies(${target} bsfNullRenderAPI)
		endif()

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

	add_dependencies(${target} bsfSL)
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
		if(NOT ${isInProjectFolder})
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

# Sets up default linker and compiler flags for the provided target, for each configuration.
function(B3DSetDefaultLinkAndCompileFlags target)
	get_target_property(target_type ${target} TYPE)

	if(MSVC)
		# Linker
		# The VS generator seems picky about how the linker flags are passed: we have to make sure
		# the options are quoted correctly and with append_string or random semicolons will be
		# inserted in the command line; and unrecognised options are only treated as warnings
		# and not errors so they won't be caught by CI. Make sure the options are separated by
		# spaces too.
		# For some reason this does not apply to the compiler options...

		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS "/DYNAMICBASE /NOLOGO")

		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_DEBUG "/DEBUG")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO "/DEBUG /LTCG:incremental /INCREMENTAL:NO /OPT:REF")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")

		if(B3D_IS_64BIT)
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO " /OPT:ICF")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL " /OPT:ICF")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE " /OPT:ICF")
		endif()

		if (${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "MODULE_LIBRARY")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /DLL")
		endif()

		# Compiler
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /GS- /W3 /WX- /MP /fp:fast /fp:except- /nologo /bigobj /wd4577 /wd4530)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -DWIN32 -D_WINDOWS)

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Od /RTC1 /MDd -DDEBUG>)

		if(B3D_IS_64BIT) # Debug edit and continue for 64-bit
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/ZI>)
		else() # Normal debug for 32-bit
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Zi>)
		endif()

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:/GL /Gy /Zi /O2 /Oi /MD -DDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:/GL /Gy /Zi /O2 /Oi /MD -DNDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:/GL /Gy /Zi /O2 /Oi /MD -DNDEBUG>)

		# Global defines
		#add_definitions(-D_HAS_EXCEPTIONS=0)

	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		# Note: Optionally add -ffunction-sections, -fdata-sections, but with linker option --gc-sections
		# TODO: Use link-time optimization -flto. Might require non-default linker.
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-unused-parameter -Wno-reorder-ctor -fPIC -fno-strict-aliasing -msse4.1 -ffast-math)

		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fno-ms-compatibility)

			if(APPLE)
				set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fobjc-arc $<$<COMPILE_LANGUAGE:CXX>:-std=c++1z>)
			endif()
		endif()

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:-ggdb -O0 -DDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:-ggdb -O2 -DDEBUG -Wno-unused-variable> -fveclib=libmvec)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:-ggdb -O2 -DNDEBUG -Wno-unused-variable> -fveclib=libmvec)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:-ggdb -O2 -DNDEBUG -Wno-unused-variable> -fveclib=libmvec)

		if (${target_type} STREQUAL "EXECUTABLE")
			if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_DEBUG -no-pie)
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO -no-pie)
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_MINSIZEREL -no-pie)
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELEASE -no-pie)
			endif()
		endif()
	else()
		# TODO_OTHER_COMPILERS_GO_HERE
	endif()

	if (${target_type} STREQUAL "SHARED_LIBRARY")
		set_property(TARGET ${target} PROPERTY VERSION ${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH})
		set_property(TARGET ${target} PROPERTY SOVERSION ${B3D_FRAMEWORK_VERSION_MAJOR})
	endif()

	if(APPLE)
		set_property(TARGET ${target} PROPERTY INSTALL_RPATH "@loader_path;@loader_path/../lib;@loader_path/bsf-${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH}")
	else()
		if (${target_type} STREQUAL "EXECUTABLE")
			set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/../lib/bsf-${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH}")
		else()
			set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN:\$ORIGIN/bsf-${B3D_FRAMEWORK_VERSION_MAJOR}.${B3D_FRAMEWORK_VERSION_MINOR}.${B3D_FRAMEWORK_VERSION_PATCH}")
		endif()
	endif()
endfunction()