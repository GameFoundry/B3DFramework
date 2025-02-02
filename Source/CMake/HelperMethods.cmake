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
		find_path(EXAMPLE_SUBMODULE_SOURCES "CMakeLists.txt" PATHS "${BSF_DIRECTORY}/Examples" NO_DEFAULT_PATH NO_CACHE)
		if(NOT EXAMPLE_SUBMODULE_SOURCES)
			execute_process(COMMAND git submodule update
					--init
					-- Examples
					WORKING_DIRECTORY ${BSF_DIRECTORY})
		endif()
		mark_as_advanced(EXAMPLE_SUBMODULE_SOURCES)

		add_subdirectory(${BSF_DIRECTORY}/Examples)
	endif()

	# Grab code-generator project
	if(B3D_BUILD_CODEGEN)
		find_path(CODEGEN_SUBMODULE_SOURCES "CMakeLists.txt" PATHS "${BSF_TOOLS_DIRECTORY}/BansheeCodeGenerator" NO_DEFAULT_PATH NO_CACHE)
		if(NOT CODEGEN_SUBMODULE_SOURCES)
			execute_process(COMMAND git submodule update
					--init
					-- BansheeCodeGenerator
					WORKING_DIRECTORY ${BSF_TOOLS_DIRECTORY})
		endif()
		mark_as_advanced(CODEGEN_SUBMODULE_SOURCES)

		add_subdirectory(${BSF_TOOLS_DIRECTORY}/BansheeCodeGenerator)
		set_property(TARGET BansheeCodeGenerator PROPERTY FOLDER Tools)
	endif()

	# Script binding generation script
	if(((SCRIPT_API AND (NOT SCRIPT_API MATCHES "None")) OR B3D_IS_ENGINE) AND B3D_BUILD_CODEGEN)
		include(${BSF_SOURCE_DIR}/CMake/GenerateScriptBindings.cmake)
	endif()

	# Stock icon generation script
	include(${BSF_SOURCE_DIR}/CMake/StockIcons.cmake)
endfunction()

function(add_prefix var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
endfunction()

function(add_engine_dependencies target_name)
	add_engine_dependencies2(${target_name} ${B3D_BUILD_ALL_RENDER_BACKENDS})
endfunction()

function(add_engine_dependencies2 target_name all_render_api)
	if(${all_render_api})
		if(WIN32)
			add_dependencies(${target_name} bsfD3D11RenderAPI bsfVulkanRenderAPI bsfGLRenderAPI bsfNullRenderAPI)
		elseif(APPLE)
			add_dependencies(${target_name} bsfGLRenderAPI bsfNullRenderAPI)
		else()
			add_dependencies(${target_name} bsfVulkanRenderAPI bsfGLRenderAPI bsfNullRenderAPI)
		endif()
	else()
		if(B3D_RENDER_BACKEND MATCHES "DirectX 11")
			add_dependencies(${target_name} bsfD3D11RenderAPI)
		elseif(B3D_RENDER_BACKEND MATCHES "Vulkan")
			add_dependencies(${target_name} bsfVulkanRenderAPI)
		elseif(B3D_RENDER_BACKEND MATCHES "OpenGL")
			add_dependencies(${target_name} bsfGLRenderAPI)
		else()
			add_dependencies(${target_name} bsfNullRenderAPI)
		endif()
	endif()

	if(B3D_AUDIO_BACKEND MATCHES "FMOD")
		add_dependencies(${target_name} bsfFMOD)
	elseif(B3D_AUDIO_BACKEND MATCHES "OpenAudio")
		add_dependencies(${target_name} bsfOpenAudio)
	else()
		add_dependencies(${target_name} bsfNullAudio)
	endif()
	
	if(B3D_PHYSICS_BACKEND MATCHES "PhysX")
		add_dependencies(${target_name} bsfPhysX)
	else()
		add_dependencies(${target_name} bsfNullPhysics)
	endif()
	
	if(B3D_RENDERER MATCHES "RenderBeast")
		add_dependencies(${target_name} bsfRenderBeast)
	else()
		add_dependencies(${target_name} bsfNullRenderer)
	endif()
	
	add_dependencies(${target_name} bsfSL)
endfunction()

function(add_importer_dependencies target_name)
	add_dependencies(${target_name} bsfFBXImporter bsfFontImporter bsfFreeImgImporter bsfSL)
endfunction()

function(add_subdirectory_optional subdir_name)
	get_filename_component(fullPath ${subdir_name} ABSOLUTE)
	if(EXISTS ${fullPath}/CMakeLists.txt)
		add_subdirectory(${subdir_name})
	endif()
endfunction()

function(target_link_framework TARGET FRAMEWORK)
	find_library(FM_${FRAMEWORK} ${FRAMEWORK})

	if(NOT FM_${FRAMEWORK})
		message(FATAL_ERROR "Cannot find ${FRAMEWORK} framework.")
	endif()

	target_link_libraries(${TARGET} PRIVATE ${FM_${FRAMEWORK}})
	mark_as_advanced(FM_${FRAMEWORK})
endfunction()

set(BS_BINARY_DEP_WEBSITE "https://aigaion.feralhosting.com/bearishsun/banshee" CACHE STRING "The location that binary dependencies will be pulled from.")
mark_as_advanced(BS_BINARY_DEP_WEBSITE)

function(update_binary_deps DEP_PREFIX DEP_NAME DEP_FOLDER DEP_VERSION)
	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)	

	if(WIN32)
		set(DEP_TYPE VS2015)
	elseif(LINUX)
		set(DEP_TYPE Linux)
	elseif(APPLE)
		set(DEP_TYPE macOS)
	endif()

	set(BINARY_DEPENDENCIES_URL ${BS_BINARY_DEP_WEBSITE}/${DEP_PREFIX}_${DEP_TYPE}_Master_${DEP_VERSION}.zip)
	file(DOWNLOAD ${BINARY_DEPENDENCIES_URL} ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
		SHOW_PROGRESS
		STATUS DOWNLOAD_STATUS)
		
	list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
	if(NOT STATUS_CODE EQUAL 0)
		message(FATAL_ERROR "Binary dependencies failed to download from URL: ${BINARY_DEPENDENCIES_URL}")
	endif()
	
	message(STATUS "Extracting files. Please wait...")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Temp
	)
	
	# Copy executables and dynamic libraries
	if(EXISTS ${PROJECT_SOURCE_DIR}/Temp/bin)
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/bin ${DEP_FOLDER}/../bin)	
	endif()
	
	# Copy static libraries, headers and tools
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER})	
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/${DEP_NAME} ${DEP_FOLDER})
	
	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
endfunction()

function(check_and_update_binary_deps DEP_PREFIX DEP_NAME DEP_FOLDER DEP_VERSION)
	set(BUILTIN_DEP_VERSION_FILE ${DEP_FOLDER}/.version)
	if(NOT EXISTS ${BUILTIN_DEP_VERSION_FILE})
		message(STATUS "Binary dependencies for '${DEP_PREFIX}' are missing. Downloading package...")
		update_binary_deps(${DEP_PREFIX} ${DEP_NAME} ${DEP_FOLDER} ${DEP_VERSION})
	else()
		file (STRINGS ${BUILTIN_DEP_VERSION_FILE} CURRENT_DEP_VERSION)
		if(${DEP_VERSION} GREATER ${CURRENT_DEP_VERSION})
			message(STATUS "Your precomiled dependencies package for '${DEP_PREFIX}' is out of date. Downloading latest package...")
			update_binary_deps(${DEP_PREFIX} ${DEP_NAME} ${DEP_FOLDER} ${DEP_VERSION})
		endif()
	endif()
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

function(copy_folder_on_build target srcDir dstDir name filter)
	set(SRC_DIR ${srcDir}/${name})
	set(DST_DIR ${dstDir}/${name})
	
	file(GLOB_RECURSE ALL_FILES RELATIVE ${SRC_DIR} "${SRC_DIR}/${filter}")

	foreach(CUR_PATH ${ALL_FILES})
		get_filename_component(FILENAME ${CUR_PATH} NAME)
	
		set(SRC ${SRC_DIR}/${CUR_PATH})
		set(DST ${DST_DIR}/${CUR_PATH})
		add_custom_command(
		   TARGET ${target} POST_BUILD
		   COMMAND ${CMAKE_COMMAND}
		   ARGS    -E copy_if_different ${SRC} ${DST}
		   COMMENT "Copying ${SRC} ${DST}"
		   )
	endforeach()
endfunction()

function(add_common_flags target)
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
##################### Built-in asset import and versioning ############################
#######################################################################################

function(check_for_changes2 _FOLDER _FILTER _TIMESTAMP _IS_CHANGED)
	file(GLOB_RECURSE ALL_FILES "${_FOLDER}/${_FILTER}")
	foreach(CUR_FILE ${ALL_FILES})
		file(TIMESTAMP ${CUR_FILE} FILE_TIMESTAMP %s)
		if(${FILE_TIMESTAMP} GREATER ${_TIMESTAMP})
			set(${_IS_CHANGED} ON PARENT_SCOPE)
			return()
		endif()
	endforeach()
	
	set(${_IS_CHANGED} OFF PARENT_SCOPE)
endfunction()

function(check_for_changes _FOLDER _FILTER _TIMESTAMP_FILE IS_CHANGED)
	file(TIMESTAMP ${_TIMESTAMP_FILE} _TIMESTAMP %s)
	check_for_changes2(${_FOLDER} ${_FILTER} ${_TIMESTAMP} IS_CHANGED_FOLDER)
	set(${IS_CHANGED} ${IS_CHANGED_FOLDER} PARENT_SCOPE)
endfunction()

function(update_builtin_assets ASSET_PREFIX ASSET_FOLDER FOLDER_NAME ASSET_VERSION CLEAR_MANIFEST)
	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)	
	
	set(ASSET_DEPENDENCIES_URL ${BS_BINARY_DEP_WEBSITE}/${ASSET_PREFIX}Data_Master_${ASSET_VERSION}.zip)
	file(DOWNLOAD ${ASSET_DEPENDENCIES_URL} ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
		SHOW_PROGRESS
		STATUS DOWNLOAD_STATUS)
		
	list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
	if(NOT STATUS_CODE EQUAL 0)
		message(FATAL_ERROR "Builtin assets failed to download from URL: ${ASSET_DEPENDENCIES_URL}")
	endif()
	
	message(STATUS "Extracting files. Please wait...")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xzf ${PROJECT_SOURCE_DIR}/Temp/Dependencies.zip
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Temp
	)
	
	# Copy files
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/${FOLDER_NAME} ${ASSET_FOLDER})
	
	# Make sure timestamp modify date/times are newer (avoids triggering reimport)
	execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${ASSET_FOLDER}/Timestamp.asset )
	
	# Make sure resource manifests get rebuilt
	if(CLEAR_MANIFEST)
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${ASSET_FOLDER}/ResourceManifest.asset)
	endif()
	
	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
endfunction()

function(check_and_update_builtin_assets ASSET_PREFIX ASSET_FOLDER FOLDER_NAME ASSET_VERSION CLEAR_MANIFEST)
	set(BUILTIN_ASSETS_VERSION_FILE ${ASSET_FOLDER}/.version)
	if(NOT EXISTS ${BUILTIN_ASSETS_VERSION_FILE})
		message(STATUS "Builtin assets for '${ASSET_PREFIX}' are missing. Downloading package...")
		update_builtin_assets(${ASSET_PREFIX} ${ASSET_FOLDER} ${FOLDER_NAME} ${ASSET_VERSION} ${CLEAR_MANIFEST})	
	else()
		file (STRINGS ${BUILTIN_ASSETS_VERSION_FILE} CURRENT_BUILTIN_ASSET_VERSION)
		if(${ASSET_VERSION} GREATER ${CURRENT_BUILTIN_ASSET_VERSION})
			message(STATUS "Your builtin asset package for '${ASSET_PREFIX}' is out of date. Downloading latest package...")
			update_builtin_assets(${ASSET_PREFIX} ${ASSET_FOLDER} ${FOLDER_NAME} ${ASSET_VERSION} ${CLEAR_MANIFEST})	
		endif()
	endif()
endfunction()

function(add_run_asset_import_target _PREFIX _FOLDER _WORKING_DIR _ARGS)
	find_package(bsfImportTool)

	if(NOT bsfImportTool_FOUND)
		message("Cannot add asset import target because bsfImportTool binaries cannot be found. Build the bsfImportTool target and install it to Dependencies/tools/bsfImportTool.")
	else()
		set(RunAssetImport_EXECUTABLE ${bsfImportTool_EXECUTABLE})
		set(RunAssetImport_INPUT_FOLDER ${_FOLDER})
		set(RunAssetImport_CMD_ARGS ${_ARGS})
		set(RunAssetImport_PREFIX ${_PREFIX})
		set(RunAssetImport_WORKING_DIR ${_WORKING_DIR})
		
		configure_file(
			${BSF_SOURCE_DIR}/CMake/Scripts/RunAssetImport.cmake.in
			${CMAKE_CURRENT_BINARY_DIR}/RunAssetImport_${_PREFIX}.cmake
			@ONLY)
		
		add_custom_target(RunAssetImport_${_PREFIX} COMMAND ${CMAKE_COMMAND} -P
			${CMAKE_CURRENT_BINARY_DIR}/RunAssetImport_${_PREFIX}.cmake)
			
		set_property(TARGET RunAssetImport_${_PREFIX} PROPERTY FOLDER Scripts)
	endif()
endfunction()

function(add_upload_assets_target _PREFIX _NAME _FOLDER _FILES)
	set(UploadAssets_FTP_CREDENTIALS_FILE ${BS_FTP_CREDENTIALS_FILE})
	set(UploadAssets_INPUT_FOLDER ${_FOLDER})
	set(UploadAssets_INPUT_FILES ${_FILES})
	set(UploadAssets_ARCHIVE_NAME ${_NAME})
	set(UploadAssets_TEMP_FOLDER ${PROJECT_SOURCE_DIR}/Temp)

	configure_file(
		${BSF_SOURCE_DIR}/CMake/Scripts/UploadAssets.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/UploadAssets_${_PREFIX}.cmake
		@ONLY)
	
	add_custom_target(UploadAssets_${_PREFIX} COMMAND ${CMAKE_COMMAND} -P
		${CMAKE_CURRENT_BINARY_DIR}/UploadAssets_${_PREFIX}.cmake)
		
	set_property(TARGET UploadAssets_${_PREFIX} PROPERTY FOLDER Scripts)
endfunction()

#######################################################################################
##################### Precompiled header (Cotire) related #############################
#######################################################################################

function(find_clang_invalid_libc_pch_headers banned_files)
	if (NOT UNIX)
		return()
	endif()

	if (CMAKE_C_COMPILER_ID MATCHES "Clang")
		execute_process(COMMAND ${CMAKE_C_COMPILER} -E -x c - -v
						INPUT_FILE /dev/null
						OUTPUT_FILE /dev/null
			            ERROR_VARIABLE clang_c_raw_verbose)

		string(REGEX MATCHALL "\n /[^\n]*" clang_c_search_dirs "${clang_c_raw_verbose}")
		string(REPLACE "\n " "" clang_c_search_dirs "${clang_c_search_dirs}")

		find_file(inttypes_c_location "inttypes.h" PATHS ${clang_c_search_dirs} PATH_SUFFIXES include NO_DEFAULT_PATH)
	endif()

	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		execute_process(COMMAND ${CMAKE_CXX_COMPILER} -E -x c++ - -v
						INPUT_FILE /dev/null
						OUTPUT_FILE /dev/null
			            ERROR_VARIABLE clang_cxx_raw_verbose)

		string(REGEX MATCHALL "\n /[^\n]*" clang_cxx_search_dirs "${clang_cxx_raw_verbose}")
		string(REPLACE "\n " "" clang_cxx_search_dirs "${clang_cxx_search_dirs}")

		find_file(inttypes_cxx_location "inttypes.h" PATHS ${clang_cxx_search_dirs} PATH_SUFFIXES include NO_DEFAULT_PATH)
	endif()
	set(${banned_files} ${inttypes_c_location} ${inttypes_cxx_location} PARENT_SCOPE)
endfunction()

function(find_windows_system_headers paths files_to_find)
	if (NOT MSVC)
		return()
	endif()

	set(file_string "int main(){}\n")

	foreach (file ${files_to_find})
		set(file_string "${file_string}\n#include<${file}>")
	endforeach()

	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/default_headers.h.cxx"
	           ${file_string})

	if (COTIRE_DEBUG)
		message(STATUS "Compiling the following file to find ${files_to_find}: " ${file_string})
	endif()

	try_compile(try_compile_result "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/default_headers.h.cxx"
            COMPILE_DEFINITIONS /showIncludes
            OUTPUT_VARIABLE try_compile_output
            )

	if (COTIRE_DEBUG)
		message(STATUS "${try_compile_output}")
	endif()

	string(REGEX MATCHALL "Note: including file:\\w*([^\n]*)" include_list "${try_compile_output}")

	set(to_return "")

	foreach(entry ${include_list})
		string(REGEX REPLACE "Note: including file:\\w*([^\n]*)" "\\1" entry_path "${entry}")
		foreach (file ${files_to_find})
			if (${entry_path} MATCHES "${file}")
				string(STRIP ${entry_path} entry_path_stripped)
				file(TO_CMAKE_PATH "${entry_path_stripped}" entry_path_stripped_slashes)
				set(to_return ${to_return} ${entry_path_stripped_slashes})
			endif()
		endforeach()
	endforeach()

	set(${paths} ${to_return} PARENT_SCOPE)
endfunction()

macro(conditional_cotire)
	if(COMMAND cotire)
		cotire(${ARGN})
	endif()
endmacro()