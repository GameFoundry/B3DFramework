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
	if(((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE) AND B3D_BUILD_CODEGEN)
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
	if(${B3D_BUILD_ALL_RENDER_BACKENDS})
		if(WIN32)
			add_dependencies(${target} bsfVulkanRenderAPI bsfNullRenderAPI)
		elseif(APPLE)
			add_dependencies(${target} bsfVulkanRenderAPI bsfNullRenderAPI)
		else()
			add_dependencies(${target} bsfVulkanRenderAPI bsfNullRenderAPI)
		endif()
	else()
		if(B3D_RENDER_BACKEND MATCHES "Vulkan")
			add_dependencies(${target} bsfVulkanRenderAPI)
		else()
			add_dependencies(${target} bsfNullRenderAPI)
		endif()
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
	if(UNIX AND BSF_STRIP_DEBUG_INFO)
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

# Copies the provided follow when @p target is built.
#
# @param	target					Target which needs to build to trigger the copy operation.
# @param	sourceParentFolder		Location containing the folder to copy from.
# @param	destinationParentFolder	Location to copy the folder to.
# @param	folderName				Name of the folder to copy.
# @param	filter					Filter to copy only certain files, *.* to copy all files.
function(B3DCopyFolderOnBuild target sourceParentFolder destinationParentFolder folderName filter)
	set(sourceFolder ${sourceParentFolder}/${folderName})
	set(destinationFolder ${destinationParentFolder}/${folderName})
	
	file(GLOB_RECURSE allFiles RELATIVE ${sourceFolder} "${destinationFolder}/${filter}")

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

#######################################################################################
############################## Find package helpers ###################################
#######################################################################################

# Marks the beginning of a CMake Find* script, to be used with the find_package() command.
#
# @param	packageName		Name of the package that's being located.
MACRO(B3DStartFindPackage packageName)
	message(STATUS "Looking for ${packageName} installation...")
ENDMACRO()

# Marks the end of CMake Find* script.
#
# @param	packageName			Name of the package that's being located.
# @param	mainLibraryName		Name of the primary library in the package, without an extension.
MACRO(B3DEndFindPackage packageName mainLibraryName)
	if(NOT ${packageName}_FOUND)
		if(${packageName}_FIND_REQUIRED)
			message(FATAL_ERROR "Cannot find ${packageName} installation. Try modifying the ${packageName}_INSTALL_DIR path.")
		elseif(NOT ${packageName}_FIND_QUIETLY)
			message(WARNING "Cannot find ${packageName} installation. Try modifying the ${packageName}_INSTALL_DIR path.")
		endif()
	else()
		set_target_properties(${packageName}::${mainLibraryName} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${${packageName}_INCLUDE_DIR}")
		message(STATUS "...${packageName} OK.")
	endif()

	mark_as_advanced(${packageName}_INSTALL_DIR ${packageName}_INCLUDE_DIR)
	set(${packageName}_INCLUDE_DIRS ${${packageName}_INCLUDE_DIR})
ENDMACRO()

# Sets up default paths in which to look for library includes and binaries. These paths will be used internally
# by B3DFindImported* family of methods, so you should call this before calling these methods. User can also manually
# add paths to the variables defined by this command.
#
# @param	packageName		Name of the package being located.
#
# Must be defined before calling:
#  - ${packageName}_INSTALL_DIR - Path to the location of package, relative to which the search paths will be set up.
#
# Will define:
#  - ${packageName}_INCLUDE_SEARCH_DIRS
#  - ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS
#  - ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS
#  - ${packageName}_BINARY_RELEASE_SEARCH_DIRS
#  - ${packageName}_BINARY_DEBUG_SEARCH_DIRS
MACRO(B3DPopulateDefaultPackageSearchPaths packageName)
	set(${packageName}_INCLUDE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/include")

	if (B3D_IS_64BIT)
		set(platform "x64")
	else ()
		set(platform "x86")
	endif ()

	# Both platform and configuration specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}/Release")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}/Debug")

	# Only platform specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/${platform}")

	# Only configuration specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/Release")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib/Debug")

	# Neither platform and configuration specified (Library)
	list(APPEND ${packageName}_LIBRARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib")
	list(APPEND ${packageName}_LIBRARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/lib")

	# Both platform and configuration specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}/Release")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}/Debug")

	# Only platform specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/${platform}")

	# Only configuration specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/Release")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin/Debug")

	# Neither platform and configuration specified (Binary)
	list(APPEND ${packageName}_BINARY_RELEASE_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin")
	list(APPEND ${packageName}_BINARY_DEBUG_SEARCH_DIRS "${${packageName}_INSTALL_DIR}/bin")
ENDMACRO()

# Creates a new library of IMPORTED type and sets up the external paths to library binaries.
#
# @param 		libraryName				Name of the library target that will be defined.
# @param		libraryType				Type of the library: STATIC, SHARED or MODULE.
# @param		releaseLibraryPath		Path to the library (.lib, .so, .dylib) to be used in Release configurations.
# @param		debugLibraryPath		Path to the library (.lib, .so, .dylib) to be used in Debug configurations.
# @param		releaseBinaryPath		Path to the binary (.dll) to be used in Release configurations. Only relevant on Windows.
# @param		debugBinaryPath			Path to the binary (.dll) to be used in Debug configurations. Only relevant on Windows.
#
# Will define:
# - A library named ${libraryName}
MACRO(B3DAddImportedLibrary libraryName libraryType releaseLibraryPath debugLibraryPath releaseBinaryPath debugBinaryPath)

	if(${libraryType} STREQUAL "STATIC")
		add_library(${libraryName} STATIC IMPORTED)

		set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseLibraryPath}")
		set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugLibraryPath}")
	elseif(${libraryType} STREQUAL "SHARED")
		add_library(${libraryName} SHARED IMPORTED)

		if(WIN32)
			set_target_properties(${libraryName} PROPERTIES IMPORTED_IMPLIB "${releaseLibraryPath}") # .lib
			set_target_properties(${libraryName} PROPERTIES IMPORTED_IMPLIB_DEBUG "${debugLibraryPath}") # .lib
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseBinaryPath}") # .dll
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugBinaryPath}") # .dll
		else()
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseLibraryPath}")
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugLibraryPath}")
		endif()
	elseif(${libraryType} STREQUAL "MODULE")

		if(WIN32)
			add_library(${libraryName} INTERFACE IMPORTED)

			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseBinaryPath}") # .dll
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugBinaryPath}") # .dll
		else()
			add_library(${libraryName} MODULE IMPORTED)

			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION "${releaseLibraryPath}")
			set_target_properties(${libraryName} PROPERTIES IMPORTED_LOCATION_DEBUG "${debugLibraryPath}")
		endif()
	else()
		message(ERROR "Invalid library type: ${libraryType}")
	endif()
ENDMACRO()

# Attempts to locate a library using the paths set up by B3DPopulateDefaultPackageSearchPaths().
#
# @param	packageName				Name of the package that the library is a part of.
# @param 	libraryName				Name of the library target that will be defined.
# @param	libraryType				Type of the library: STATIC, SHARED or MODULE.
# @param	releaseLibraryName		Name of the library (.lib, .so, .dylib) to be used for Release configurations (without an extension).
# @param	debugLibraryName		Name of the library (.lib, .so, .dylib) to be used for Debug configurations (without an extension).
# @param	releaseBinaryName		Name of the binary (.dll) to be used for Release configurations (without an extension). Only relevant on Windows.
# @param	debugBinaryName			Name of the binary (.dll) to be used for Debug configurations (without an extension). Only relevant on Windows.
#
# Will define:
# - A library named ${packageName}::${libraryName}
# - Library will be appended to list ${packageName}_LIBRARIES
MACRO(B3DFindImportedLibraryWithExplicitNames packageName libraryName libraryType releaseLibraryName debugLibraryName releaseBinaryName debugBinaryName)

	if(NOT ${libraryType} STREQUAL "MODULE" OR NOT WIN32)
		find_library(${libraryName}_LIBRARY_RELEASE NAMES ${releaseLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
		find_library(${libraryName}_LIBRARY_RELEASE NAMES ${releaseLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS})

		if(${packageName}_LIBRARY_DEBUG_SEARCH_DIRS)
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_DEBUG_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_DEBUG_SEARCH_DIRS})
		else()
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_library(${libraryName}_LIBRARY_DEBUG NAMES ${debugLibraryName} PATHS ${${packageName}_LIBRARY_RELEASE_SEARCH_DIRS})
		endif()

		if(NOT ${libraryName}_LIBRARY_RELEASE OR NOT ${libraryName}_LIBRARY_DEBUG)
			set(${packageName}_FOUND FALSE)
			message(STATUS "...Cannot find imported library: ${libraryName}")
		endif()
	endif()

	if(NOT ${libraryType} STREQUAL "STATIC" AND WIN32)
		find_file(${libraryName}_BINARY_RELEASE NAMES ${releaseBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
		find_file(${libraryName}_BINARY_RELEASE NAMES ${releaseBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS})

		if(${packageName}_BINARY_DEBUG_SEARCH_DIRS)
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_DEBUG_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_DEBUG_SEARCH_DIRS})
		else()
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
			find_file(${libraryName}_BINARY_DEBUG NAMES ${debugBinaryName}.dll PATHS ${${packageName}_BINARY_RELEASE_SEARCH_DIRS})
		endif()

		if(NOT ${libraryName}_BINARY_RELEASE OR NOT ${libraryName}_BINARY_DEBUG)
			set(${packageName}_FOUND FALSE)
			message(STATUS "...Cannot find imported library .dll: ${libraryName}")
		endif()
	endif()

	if(NOT ${packageName}_FOUND OR ${${packageName}_FOUND})
		B3DAddImportedLibrary(${packageName}::${libraryName} ${libraryType} "${${libraryName}_LIBRARY_RELEASE}" "${${libraryName}_LIBRARY_DEBUG}" "${${libraryName}_BINARY_RELEASE}" "${${libraryName}_BINARY_DEBUG}")
	endif()

	list(APPEND ${packageName}_LIBRARIES ${packageName}::${libraryName})
	mark_as_advanced(${libraryName}_LIBRARY_RELEASE ${libraryName}_LIBRARY_DEBUG)
	mark_as_advanced(${libraryName}_BINARY_RELEASE ${libraryName}_BINARY_DEBUG)
ENDMACRO()

# Equivalent to B3DFindImportedLibraryWithExplicitNames(), but uses @p libraryName for all library file names as a convenience.
MACRO(B3DFindImportedLibrary packageName libraryName libraryType)
	B3DFindImportedLibraryWithExplicitNames(${packageName} ${libraryName} ${libraryType} ${libraryName} ${libraryName} ${libraryName} ${libraryName})
ENDMACRO()

# Equivalent to B3DFindImportedLibrary(), but allows you to provide separate file names for Release and Debug configurations.
MACRO(B3DFindImportedLibraryWithConfigurationNames packageName libraryName libraryType releaseLibraryFileName debugLibraryFileName)
	B3DFindImportedLibraryWithExplicitNames(${packageName} ${libraryName} ${libraryType} ${releaseLibraryFileName} ${debugLibraryFileName} ${releaseLibraryFileName} ${debugLibraryFileName})
ENDMACRO()

# Equivalent to B3DFindImportedLibrary(), but allows you to provide an alternate filename for .dll files. Relevant on Windows only.
MACRO(B3DFindImportedLibraryWithAlternateBinaryName packageName libraryName libraryType binaryFileName)
	B3DFindImportedLibraryWithExplicitNames(${packageName} ${libraryName} ${libraryType} ${libraryName} ${libraryName} ${binaryFileName} ${binaryFileName})
ENDMACRO()

# Looks for include files for an imported library. Uses paths set up by a previous call to B3DPopulateDefaultPackageSearchPaths()
#
# @param	packageName		Name of the package that the includes are a part of.
# @param	includePath		Relative path to an include file to use as a reference for finding the includes.
MACRO(B3DFindImportedIncludes packageName includePath)
	find_path(${packageName}_INCLUDE_DIR NAMES ${includePath} PATHS ${${packageName}_INCLUDE_SEARCH_DIRS} NO_DEFAULT_PATH)
	find_path(${packageName}_INCLUDE_DIR NAMES ${includePath} PATHS ${${packageName}_INCLUDE_SEARCH_DIRS})

	if(${packageName}_INCLUDE_DIR)
		set(${packageName}_FOUND TRUE)
	else()
		message(STATUS "...Cannot find include file \"${includePath}\" at path ${${packageName}_INCLUDE_SEARCH_DIRS}")
		set(${packageName}_FOUND FALSE)
	endif()
ENDMACRO()

#######################################################################################
######################## Pre-built dependency download ################################
#######################################################################################

set(B3D_PREBUILT_DEPENDENCIES_URL "https://aigaion.feralhosting.com/bearishsun/banshee" CACHE STRING "The location that binary dependencies will be pulled from.")
mark_as_advanced(B3D_PREBUILT_DEPENDENCIES_URL)

# Downloads prebuilt binary dependencies and copies them into the dependencies folder.
#
# @param	dependencyPrefix 	Prefix identifying the dependency pack we're downloading (e.g. 'Framework', 'Editor', etc.)
# @param	dependencyFolder	Folder in which to unpack the dependencies, e.g. '${PROJECT_SOURCE_DIR}/Dependencies'.
# @param	folderName			Name of the folder in the downloaded package in which data is stored. Usually the same
#								as the last folder in @p dependencyFolder (e.g. 'Dependencies').
# @param	dependencyVersion	Version of the dependencies to download.
function(B3DUpdatePrebuiltDependencies dependencyPrefix dependencyFolder folderName dependencyVersion)
	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)

	if(WIN32)
		set(dependencyType VS2015)
	elseif(LINUX)
		set(dependencyType Linux)
	elseif(APPLE)
		set(dependencyType macOS)
	endif()

	set(binaryDependenciesURL ${B3D_PREBUILT_DEPENDENCIES_URL}/${dependencyPrefix}_${dependencyType}_Master_${dependencyVersion}.zip)
	file(DOWNLOAD ${binaryDependenciesURL} ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
			SHOW_PROGRESS
			STATUS DOWNLOAD_STATUS)

	list(GET DOWNLOAD_STATUS 0 statusCode)
	if(NOT statusCode EQUAL 0)
		message(FATAL_ERROR "Binary dependencies failed to download from URL: ${binaryDependenciesURL}")
	endif()

	message(STATUS "Extracting files. Please wait...")
	execute_process(
			COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Temp
	)

	# Copy executables and dynamic libraries
	if(EXISTS ${PROJECT_SOURCE_DIR}/Temp/bin)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/bin ${dependencyFolder}/../bin)
	endif()

	# Copy static libraries, headers and tools
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${dependencyFolder})
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/${folderName} ${dependencyFolder})

	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)
endfunction()

# Checks if dependencies are out of date and if so, downloads prebuilt binary dependencies and copies them into the
# dependencies folder.
#
# @param	dependencyPrefix 	Prefix identifying the dependency pack we're downloading (e.g. 'Framework', 'Editor', etc.)
# @param	dependencyFolder	Folder in which to unpack the dependencies, e.g. '${PROJECT_SOURCE_DIR}/Dependencies'.
# @param	folderName			Name of the folder in the downloaded package in which data is stored. Usually the same
#								as the last folder in @p dependencyFolder (e.g. 'Dependencies').
# @param	dependencyVersion	Version of the dependencies to download.
function(B3DCheckAndUpdatePrebuiltDependencies dependencyPrefix dependencyFolder folderName dependencyVersion)
	set(builtinDependencyVersionFile ${dependencyFolder}/.version)
	if(NOT EXISTS ${builtinDependencyVersionFile})
		message(STATUS "Binary dependencies for '${dependencyPrefix}' are missing. Downloading package...")
		B3DUpdatePrebuiltDependencies(${dependencyPrefix} ${dependencyFolder} ${folderName} ${dependencyVersion})
	else()
		file (STRINGS ${builtinDependencyVersionFile} currentDependencyVersion)
		if(${dependencyVersion} GREATER ${currentDependencyVersion})
			message(STATUS "Your precomiled dependencies package for '${dependencyPrefix}' is out of date. Downloading latest package...")
			B3DUpdatePrebuiltDependencies(${dependencyPrefix} ${dependencyFolder} ${folderName} ${dependencyVersion})
		endif()
	endif()
endfunction()

#######################################################################################
##################### Built-in asset import and versioning ############################
#######################################################################################

# Downloads assets with the provided version and unpacks them to the assets folder, overwriting existing assets
#
# @param 	assetPrefix		Prefix identifying the asset pack we're downloading (e.g. 'Framework', 'Editor', etc.)
# @param 	assetFolder		Folder in which to unpack the assets, e.g. '${PROJECT_SOURCE_DIR}/Data'
# @param 	folderName 		Name of the folder in the downloaded package in which assets are stored. Usually the same
#							as the last folder in @p assetFolder (e.g. 'Data')
# @param	assetVersion	Version of the assets to download.
function(B3DUpdateBuiltinAssets assetPrefix assetFolder folderName assetVersion)
	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)	
	
	set(assetDependenciesURL ${B3D_PREBUILT_DEPENDENCIES_URL}/${assetPrefix}Data_Master_${assetVersion}.zip)
	file(DOWNLOAD ${assetDependenciesURL} ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
		SHOW_PROGRESS
		STATUS DOWNLOAD_STATUS)
		
	list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
	if(NOT STATUS_CODE EQUAL 0)
		message(FATAL_ERROR "Builtin assets failed to download from URL: ${assetDependenciesURL}")
	endif()
	
	message(STATUS "Extracting files. Please wait...")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Temp
	)
	
	# Copy files
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/${folderName} ${assetFolder})
	
	# Make sure timestamp modify date/times are newer (avoids triggering reimport)
	execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${assetFolder}/Timestamp.asset )

	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
endfunction()

# Checks the current version of the builtin assets, and performs update of the assets if out of date.
#
# @param 	assetPrefix		Prefix identifying the asset pack we're downloading (e.g. 'Framework', 'Editor', etc.)
# @param 	assetFolder		Folder in which to unpack the assets, e.g. '${PROJECT_SOURCE_DIR}/Data'
# @param 	folderName 		Name of the folder in the downloaded package in which assets are stored. Usually the same
#							as the last folder in @p assetFolder (e.g. 'Data')
# @param	assetVersion	Version of the assets to download.
function(B3DUpdateBuiltinAssetsIfNeeded assetPrefix assetFolder folderName assetVersion)
	set(builtinAssetVersionFile ${assetFolder}/.version)
	if(NOT EXISTS ${builtinAssetVersionFile})
		message(STATUS "Builtin assets for '${assetPrefix}' are missing. Downloading package...")
		B3DUpdateBuiltinAssets(${assetPrefix} ${assetFolder} ${folderName} ${assetVersion})
	else()
		file (STRINGS ${builtinAssetVersionFile} currentBuiltinAssetVersion)
		if(${assetVersion} GREATER ${currentBuiltinAssetVersion})
			message(STATUS "Your builtin asset package for '${assetPrefix}' is out of date. Downloading latest package...")
			B3DUpdateBuiltinAssets(${assetPrefix} ${assetFolder} ${folderName} ${assetVersion})
		endif()
	endif()
endfunction()

# Registers a target that performs asset import for builtin assets.
#
# @param	assetPrefix			Prefix identifying the asset pack to build (e.g. 'Framework', 'Editor', etc.)
# @param	inputFolder			Absolute folder in which to find raw assets to import.
# @param	outputFolder		Absolute folder in which to place imported assets.
# @param	additionalArguments	Optional additional arguments to pass to the import tool.
function(B3DAddRunAssetImportTarget assetPrefix inputFolder outputFolder additionalArguments)
	find_package(bsfImportTool)

	if(NOT bsfImportTool_FOUND)
		message("Cannot add asset import target because bsfImportTool binaries cannot be found. Build the bsfImportTool target and install it to Dependencies/tools/bsfImportTool.")
	else()
		set(RunAssetImport_EXECUTABLE ${bsfImportTool_EXECUTABLE})
		set(RunAssetImport_INPUT_FOLDER ${inputFolder})
		set(RunAssetImport_OUTPUT_FOLDER ${outputFolder})
		set(RunAssetImport_CMD_ARGS ${additionalArguments})
		set(RunAssetImport_PREFIX ${assetPrefix})

		configure_file(
			${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Scripts/RunAssetImport.cmake.in
			${CMAKE_CURRENT_BINARY_DIR}/RunAssetImport_${assetPrefix}.cmake
			@ONLY)
		
		add_custom_target(RunAssetImport_${assetPrefix} COMMAND ${CMAKE_COMMAND} -P
			${CMAKE_CURRENT_BINARY_DIR}/RunAssetImport_${assetPrefix}.cmake)
			
		set_property(TARGET RunAssetImport_${assetPrefix} PROPERTY FOLDER Scripts)
	endif()
endfunction()

# Registers a target that performs archive and upload of current set of builtin assets. Updates the .reqversion
# file to the next available version.
#
# @param	assetPrefix		Prefix identifying the asset packet to upload (e.g. 'Framework', 'Editor', etc.)
# @param	inputFolder		Folder that contains the data list file, and the .version and .reqversion files.
# @param	dataListFile	Textual file that contains a newline separated list of assets to package.
function(B3DAddUploadAssetsTarget assetPrefix inputFolder dataListFile)
	set(UploadAssets_FTP_CREDENTIALS_FILE ${B3D_FTP_CREDENTIALS_FILE})
	set(UploadAssets_INPUT_FOLDER ${inputFolder})
	set(UploadAssets_INPUT_FILES ${dataListFile})
	set(UploadAssets_ARCHIVE_NAME ${assetPrefix}Data_Master)
	set(UploadAssets_TEMP_FOLDER ${PROJECT_SOURCE_DIR}/Temp)

	configure_file(
		${B3D_FRAMEWORK_SOURCE_FOLDER}/CMake/Scripts/UploadAssets.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/UploadAssets_${assetPrefix}.cmake
		@ONLY)
	
	add_custom_target(UploadAssets_${assetPrefix} COMMAND ${CMAKE_COMMAND} -P
		${CMAKE_CURRENT_BINARY_DIR}/UploadAssets_${assetPrefix}.cmake)
		
	set_property(TARGET UploadAssets_${assetPrefix} PROPERTY FOLDER Scripts)
endfunction()