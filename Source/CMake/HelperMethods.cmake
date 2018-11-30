function(add_prefix var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
endfunction()

function(add_engine_dependencies target_name)
	add_engine_dependencies2(${target_name} FALSE)
endfunction()

function(add_engine_dependencies2 target_name all_render_api)
	if(${all_render_api})
		if(WIN32)
			add_dependencies(${target_name} bsfD3D11RenderAPI bsfVulkanRenderAPI bsfGLRenderAPI)
		elseif(APPLE)
			add_dependencies(${target_name} bsfGLRenderAPI)
		else()
			add_dependencies(${target_name} bsfVulkanRenderAPI bsfGLRenderAPI)
		endif()
	else()
		if(RENDER_API_MODULE MATCHES "DirectX 11")
			add_dependencies(${target_name} bsfD3D11RenderAPI)
		elseif(RENDER_API_MODULE MATCHES "Vulkan")
			add_dependencies(${target_name} bsfVulkanRenderAPI)
		else()
			add_dependencies(${target_name} bsfGLRenderAPI)
		endif()
	endif()

	if(AUDIO_MODULE MATCHES "FMOD")
		add_dependencies(${target_name} bsfFMOD)
	else() # Default to OpenAudio
		add_dependencies(${target_name} bsfOpenAudio)
	endif()
	
	add_dependencies(${target_name} bsfSL bsfPhysX bsfRenderBeast)
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

# Generates a set of variables pointing to the default locations for library's includes and binaries.
# Must be defined before calling:
#  - ${LIB_NAME}_INSTALL_DIR
#
# Will define:
#  - ${LIB_NAME}_INCLUDE_SEARCH_DIRS
#  - ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS
#  - ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS
MACRO(gen_default_lib_search_dirs LIB_NAME)
	set(${LIB_NAME}_INCLUDE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/include")

	if(BS_64BIT)
		list(APPEND ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x64/Release")
		list(APPEND ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x64/Debug")
	else()
		list(APPEND ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x86/Release")
		list(APPEND ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x86/Debug")
	endif()

	# Allow for paths without a configuration specified
	if(BS_64BIT)
		list(APPEND ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x64")
		list(APPEND ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x64")
	else()
		list(APPEND ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x86")
		list(APPEND ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/x86")	
	endif()
	
	# Allow for paths without a platform specified
	list(APPEND ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/Release")
	list(APPEND ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib/Debug")

	# Allow for paths without a platform or configuration specified
	list(APPEND ${LIB_NAME}_LIBRARY_RELEASE_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib")
	list(APPEND ${LIB_NAME}_LIBRARY_DEBUG_SEARCH_DIRS "${${LIB_NAME}_INSTALL_DIR}/lib")
ENDMACRO()

MACRO(add_imported_library LIB_NAME RELEASE_NAME DEBUG_NAME IS_SHARED)
	if(${IS_SHARED} AND NOT WIN32)
		add_library(${LIB_NAME} SHARED IMPORTED)
	else()
		add_library(${LIB_NAME} STATIC IMPORTED)
	endif()

	if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
		set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION_DEBUG "${DEBUG_NAME}")
		set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${RELEASE_NAME}")
		set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${RELEASE_NAME}")
		set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION_RELEASE "${RELEASE_NAME}")
	else()
		set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION "${RELEASE_NAME}")
	endif()
ENDMACRO()

MACRO(find_imported_library3 FOLDER_NAME LIB_NAME DEBUG_LIB_NAME IS_SHARED)
	find_library(${LIB_NAME}_LIBRARY_RELEASE NAMES ${LIB_NAME} PATHS ${${FOLDER_NAME}_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
	find_library(${LIB_NAME}_LIBRARY_RELEASE NAMES ${LIB_NAME} PATHS ${${FOLDER_NAME}_LIBRARY_RELEASE_SEARCH_DIRS})
	
	if(${FOLDER_NAME}_LIBRARY_DEBUG_SEARCH_DIRS)
		find_library(${LIB_NAME}_LIBRARY_DEBUG NAMES ${DEBUG_LIB_NAME} PATHS ${${FOLDER_NAME}_LIBRARY_DEBUG_SEARCH_DIRS} NO_DEFAULT_PATH)
		find_library(${LIB_NAME}_LIBRARY_DEBUG NAMES ${DEBUG_LIB_NAME} PATHS ${${FOLDER_NAME}_LIBRARY_DEBUG_SEARCH_DIRS})
	else()
		find_library(${LIB_NAME}_LIBRARY_DEBUG NAMES ${DEBUG_LIB_NAME} PATHS ${${FOLDER_NAME}_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
		find_library(${LIB_NAME}_LIBRARY_DEBUG NAMES ${DEBUG_LIB_NAME} PATHS ${${FOLDER_NAME}_LIBRARY_RELEASE_SEARCH_DIRS})
	endif()

	if(${LIB_NAME}_LIBRARY_RELEASE)
		if(${LIB_NAME}_LIBRARY_DEBUG)
			add_imported_library(${FOLDER_NAME}::${LIB_NAME} "${${LIB_NAME}_LIBRARY_RELEASE}"
				"${${LIB_NAME}_LIBRARY_DEBUG}" ${IS_SHARED})
		else()
			add_imported_library(${FOLDER_NAME}::${LIB_NAME} "${${LIB_NAME}_LIBRARY_RELEASE}"
				"${${LIB_NAME}_LIBRARY_RELEASE}" ${IS_SHARED})
		endif()
	else()
		set(${FOLDER_NAME}_FOUND FALSE)
		message(STATUS "...Cannot find imported library: ${LIB_NAME} ${${LIB_NAME}_LIBRARY_RELEASE}")
	endif()

	list(APPEND ${FOLDER_NAME}_LIBRARIES ${FOLDER_NAME}::${LIB_NAME})
	mark_as_advanced(${LIB_NAME}_LIBRARY_RELEASE ${LIB_NAME}_LIBRARY_DEBUG)
ENDMACRO()

MACRO(find_imported_library_shared2 FOLDER_NAME LIB_NAME DEBUG_LIB_NAME)
		list(APPEND ${FOLDER_NAME}_SHARED_LIBS ${LIB_NAME})
	find_imported_library3(${FOLDER_NAME} ${LIB_NAME} ${DEBUG_LIB_NAME} TRUE)
ENDMACRO()

MACRO(find_imported_library_shared FOLDER_NAME LIB_NAME)
	list(APPEND ${FOLDER_NAME}_SHARED_LIBS ${LIB_NAME})
	find_imported_library_shared2(${FOLDER_NAME} ${LIB_NAME} ${LIB_NAME})
ENDMACRO()

MACRO(find_imported_library2 FOLDER_NAME LIB_NAME DEBUG_LIB_NAME)
	find_imported_library3(${FOLDER_NAME} ${LIB_NAME} ${DEBUG_LIB_NAME} FALSE)
ENDMACRO()

MACRO(find_imported_library FOLDER_NAME LIB_NAME)
	find_imported_library2(${FOLDER_NAME} ${LIB_NAME} ${LIB_NAME})
ENDMACRO()

MACRO(find_imported_includes FOLDER_NAME INCLUDE_FILES)
	find_path(${FOLDER_NAME}_INCLUDE_DIR NAMES ${INCLUDE_FILES} PATHS ${${FOLDER_NAME}_INCLUDE_SEARCH_DIRS} NO_DEFAULT_PATH)
	find_path(${FOLDER_NAME}_INCLUDE_DIR NAMES ${INCLUDE_FILES} PATHS ${${FOLDER_NAME}_INCLUDE_SEARCH_DIRS})
	
	if(${FOLDER_NAME}_INCLUDE_DIR)
		set(${FOLDER_NAME}_FOUND TRUE)
	else()
		message(STATUS "...Cannot find include file \"${INCLUDE_FILES}\" at path ${${FOLDER_NAME}_INCLUDE_SEARCH_DIRS}")
		set(${FOLDER_NAME}_FOUND FALSE)
	endif()
ENDMACRO()

MACRO(start_find_package FOLDER_NAME)
	message(STATUS "Looking for ${FOLDER_NAME} installation...")
ENDMACRO()

MACRO(end_find_package FOLDER_NAME MAIN_LIB_NAME)
	if(NOT ${FOLDER_NAME}_FOUND)
		if(${FOLDER_NAME}_FIND_REQUIRED)
			message(FATAL_ERROR "Cannot find ${FOLDER_NAME} installation. Try modifying the ${FOLDER_NAME}_INSTALL_DIR path.")
		elseif(NOT ${FOLDER_NAME}_FIND_QUIETLY)
			message(WARNING "Cannot find ${FOLDER_NAME} installation. Try modifying the ${FOLDER_NAME}_INSTALL_DIR path.")
		endif()
	else()
		set_target_properties(${FOLDER_NAME}::${MAIN_LIB_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${${FOLDER_NAME}_INCLUDE_DIR}")
		message(STATUS "...${FOLDER_NAME} OK.")
	endif()

	mark_as_advanced(${FOLDER_NAME}_INSTALL_DIR ${FOLDER_NAME}_INCLUDE_DIR)
	set(${FOLDER_NAME}_INCLUDE_DIRS ${${FOLDER_NAME}_INCLUDE_DIR})
ENDMACRO()

MACRO(install_dependency_binaries FOLDER_NAME)
	foreach(LOOP_ENTRY ${${FOLDER_NAME}_SHARED_LIBS})
		get_filename_component(RELEASE_FILENAME ${${LOOP_ENTRY}_LIBRARY_RELEASE} NAME_WE)
		get_filename_component(DEBUG_FILENAME ${${LOOP_ENTRY}_LIBRARY_DEBUG} NAME_WE)
		
		if(WIN32)
			if(BS_64BIT)
				set(PLATFORM "x64")
			else()
				set(PLATFORM "x86")
			endif()
			
			set(RELEASE_FILENAME ${RELEASE_FILENAME}.dll)
			set(DEBUG_FILENAME ${DEBUG_FILENAME}.dll)
		
			set(SRC_RELEASE "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Release/${RELEASE_FILENAME}")
			set(SRC_DEBUG "${PROJECT_SOURCE_DIR}/bin/${PLATFORM}/Debug/${DEBUG_FILENAME}")
			set(DESTINATION_DIR bin)
		else()
			set(RELEASE_FILENAME ${RELEASE_FILENAME}.so)
			set(DEBUG_FILENAME ${DEBUG_FILENAME}.so)		
			set(SRC_RELEASE ${${LOOP_ENTRY}_LIBRARY_RELEASE})
			set(SRC_DEBUG ${${LOOP_ENTRY}_LIBRARY_DEBUG})
			set(DESTINATION_DIR lib/bsf-${BS_FRAMEWORK_VERSION_MAJOR}.${BS_FRAMEWORK_VERSION_MINOR}.${BS_FRAMEWORK_VERSION_PATCH})
		endif()
		
		install(
			FILES ${SRC_RELEASE}
			DESTINATION ${DESTINATION_DIR}
			CONFIGURATIONS Release RelWithDebInfo MinSizeRel
			RENAME ${RELEASE_FILENAME}
			OPTIONAL)
			
		install(
			FILES ${SRC_DEBUG}
			DESTINATION ${DESTINATION_DIR}
			CONFIGURATIONS Debug
			RENAME ${DEBUG_FILENAME}
			OPTIONAL)
	endforeach()
ENDMACRO()

function(target_link_framework TARGET FRAMEWORK)
	find_library(FM_${FRAMEWORK} ${FRAMEWORK})

	if(NOT FM_${FRAMEWORK})
		message(FATAL_ERROR "Cannot find ${FRAMEWORK} framework.")
	endif()

	target_link_libraries(${TARGET} PRIVATE ${FM_${FRAMEWORK}})
	mark_as_advanced(FM_${FRAMEWORK})
endfunction()

set(BS_BINARY_DEP_WEBSITE "https://data.banshee3d.com" CACHE STRING "The location that binary dependencies will be pulled from. Must follow the same naming scheme as data.banshee3d.com")
mark_as_advanced(BS_BINARY_DEP_WEBSITE)

function(update_binary_deps DEP_PREFIX DEP_FOLDER DEP_VERSION)
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

	set(BINARY_DEPENDENCIES_URL ${BS_BINARY_DEP_WEBSITE}/${DEP_PREFIX}Dependencies_${DEP_TYPE}_Master_${DEP_VERSION}.zip)
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
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/bin ${DEP_FOLDER}/../bin)	
	
	# Copy static libraries, headers and tools
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER})	
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/Dependencies ${DEP_FOLDER})
	
	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
endfunction()

function(check_and_update_binary_deps DEP_PREFIX DEP_FOLDER DEP_VERSION)
	set(BUILTIN_DEP_VERSION_FILE ${DEP_FOLDER}/.version)
	if(NOT EXISTS ${BUILTIN_DEP_VERSION_FILE})
		message(STATUS "Binary dependencies for '${DEP_PREFIX}' are missing. Downloading package...")
		update_binary_deps(${DEP_PREFIX} ${DEP_FOLDER} ${DEP_VERSION})
	else()
		file (STRINGS ${BUILTIN_DEP_VERSION_FILE} CURRENT_DEP_VERSION)
		if(${DEP_VERSION} GREATER ${CURRENT_DEP_VERSION})
			message(STATUS "Your precomiled dependencies package for '${DEP_PREFIX}' is out of date. Downloading latest package...")
			update_binary_deps(${DEP_PREFIX} ${DEP_FOLDER} ${DEP_VERSION})
		endif()
	endif()
endfunction()

function(strip_symbols targetName outputFilename)
	if(UNIX)
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

function(install_bsf_target targetName)
	strip_symbols(${targetName} symbolsFile)
	
	install(
		TARGETS ${targetName}
		EXPORT bsf
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)		
	
	if(MSVC)
		install(
			FILES $<TARGET_PDB_FILE:${targetName}> 
			DESTINATION bin 
			OPTIONAL
		)
	else()
		install(
			FILES ${symbolsFile} 
			DESTINATION lib
			OPTIONAL)
	endif()
endfunction()

function(copyBsfBinaries target srcDir)
	if(WIN32)
		set(BIN_SRC_DIR "${srcDir}/bin")
		set(BIN_DST_DIR ${PROJECT_BINARY_DIR}/bin)
		
		file(GLOB_RECURSE BIN_FILES RELATIVE ${BIN_SRC_DIR} "${BIN_SRC_DIR}/*.dll")

		foreach(CUR_PATH ${BIN_FILES})
			get_filename_component(FILENAME ${CUR_PATH} NAME)
		
			if(FILENAME MATCHES "^bsf.*")
				continue()
			endif()
		
			set(SRC ${BIN_SRC_DIR}/${CUR_PATH})
			set(DST ${BIN_DST_DIR}/${CUR_PATH})
			add_custom_command(
			   TARGET ${target} POST_BUILD
			   COMMAND ${CMAKE_COMMAND}
			   ARGS    -E copy_if_different ${SRC} ${DST}
			   COMMENT "Copying ${SRC} ${DST}\n"
			   )
		endforeach()
	endif()
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

		if(BS_64BIT)
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO " /OPT:ICF")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL " /OPT:ICF")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE " /OPT:ICF")
		endif()

		if (${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "MODULE_LIBRARY")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /DLL")
		endif()

		# Compiler
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /GS- /W3 /WX- /MP /nologo /bigobj /wd4577 /wd4530)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -DWIN32 -D_WINDOWS)

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Od /RTC1 /MDd -DDEBUG>)

		if(BS_64BIT) # Debug edit and continue for 64-bit
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
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-unused-parameter -fPIC -fno-strict-aliasing -msse4.1)

		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fno-ms-compatibility)

			if(APPLE)
				set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fobjc-arc $<$<COMPILE_LANGUAGE:CXX>:-std=c++1z>)
			endif()
		endif()

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:-ggdb -O0 -DDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:-ggdb -O2 -DDEBUG -Wno-unused-variable>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:-ggdb -O2 -DNDEBUG -Wno-unused-variable>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:-ggdb -O2 -DNDEBUG -Wno-unused-variable>)

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
		set_property(TARGET ${target} PROPERTY VERSION ${BS_FRAMEWORK_VERSION_MAJOR}.${BS_FRAMEWORK_VERSION_MINOR}.${BS_FRAMEWORK_VERSION_PATCH})
		set_property(TARGET ${target} PROPERTY SOVERSION ${BS_FRAMEWORK_VERSION_MAJOR})
	endif()

	set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN:\$ORIGIN/bsf-${BS_FRAMEWORK_VERSION_MAJOR}.${BS_FRAMEWORK_VERSION_MINOR}.${BS_FRAMEWORK_VERSION_PATCH}")
endfunction()

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

set(BS_FTP_CREDENTIALS_FILE "${PROJECT_SOURCE_DIR}/../ftp_credentials" CACHE STRING "The location containing the FTP server credentials to use for uploading packages. The file is expected to contain three lines: URL/Username/Password, in that order.")
mark_as_advanced(BS_FTP_CREDENTIALS_FILE)

function(update_data_file_package _NAME _FOLDER _FILES)
	message(STATUS "Updating data files package...")

	# Get FTP credentials needed for upload
	if(NOT EXISTS ${BS_FTP_CREDENTIALS_FILE})
		message("FTP credentials are missing at ${BS_FTP_CREDENTIALS_FILE}.")
		message(STATUS "...aborting data file packaging.")
		return()
	endif()
	
	file (STRINGS ${BS_FTP_CREDENTIALS_FILE} FTP_CREDENTIALS)

	list(GET FTP_CREDENTIALS 0 FTP_URL)
	list(GET FTP_CREDENTIALS 1 FTP_USER)
	list(GET FTP_CREDENTIALS 2 FTP_PW)
	
	# Get current version
	set(REQUESTED_VERSION_FILE ${_FOLDER}/.reqversion)
	set(CURRENT_VERSION_FILE ${_FOLDER}/.version)

	file (STRINGS ${REQUESTED_VERSION_FILE} CURRENT_VERSION_NUM)
	math(EXPR NEW_VERSION_NUM "${CURRENT_VERSION_NUM}+1")
	
	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)	

	# Update version number in the package
	file(WRITE ${CURRENT_VERSION_FILE} ${NEW_VERSION_NUM})
	
	# Generate an archive with data files
	set(ARCHIVE_FILENAME ${_NAME}_${NEW_VERSION_NUM}.zip)
	set(ARCHIVE_PATH ${PROJECT_SOURCE_DIR}/Temp/${ARCHIVE_FILENAME})

	execute_process(
		COMMAND
			${CMAKE_COMMAND} -E tar 
			"cf" "${ARCHIVE_PATH}" --format=zip
		    --files-from=${_FOLDER}/${_FILES}
		WORKING_DIRECTORY "${_FOLDER}/.."
		RESULT_VARIABLE ARCHIVE_STATUS_CODE
		ERROR_VARIABLE ARCHIVE_ERROR_MESSAGE
	)

	if(NOT ARCHIVE_STATUS_CODE EQUAL 0)
		message("Failed to create archive file: ${ARCHIVE_PATH}. Error: ${ARCHIVE_ERROR_MESSAGE}")
		message(STATUS "...aborting data file packaging.")
		return()
	endif()
	
	# Upload the package
	message(STATUS "   Uploading data package: ${ARCHIVE_FILENAME}")
	file(
		UPLOAD "${ARCHIVE_PATH}" "${FTP_URL}/${ARCHIVE_FILENAME}"
		USERPWD ${FTP_USER}:${FTP_PW}
		STATUS UPLOAD_STATUS_CODE
		SHOW_PROGRESS)
		
	if(NOT UPLOAD_STATUS_CODE EQUAL 0)
		message("Failed to upload file: ${ARCHIVE_PATH}. Error code: ${UPLOAD_STATUS_CODE}")
		message(STATUS "...aborting data file packaging.")
	endif()

	# Update active version number (only if everything else succeeds)
	file(WRITE ${REQUESTED_VERSION_FILE} ${NEW_VERSION_NUM})

	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)
	
	message(STATUS "...data file packaging succesful, new version is ${NEW_VERSION_NUM}.")
endfunction()

function(run_import_tool _PREFIX _FOLDER _ARGS)
	find_package(bsfImportTool)

	message(STATUS "Checking built-in assets for modifications for '${_PREFIX}'...")
	
	# Cannot find binaries, see if we can compile them
	if(NOT bsfImportTool_FOUND)
		message("...bsfImportTool binaries cannot be found. Build the bsfImportTool target and install it to Dependencies/tools/bsfImportTool.")
		message(STATUS "...aborting data file packaging.")
		return()
	endif()
	
	execute_process(
		COMMAND ${bsfImportTool_EXECUTABLE} ${_FOLDER}/Data/Raw/ ${_FOLDER}/Data/ ${_ARGS}
		COMMENT "...importing files, please wait."
		RESULT_VARIABLE IMPORT_STATUS_CODE
	)
	
	if((NOT IMPORT_STATUS_CODE EQUAL 0) AND (NOT IMPORT_STATUS_CODE EQUAL 1))
		message("Failed to import built-in assets. Error code: ${IMPORT_STATUS_CODE}")
		message(STATUS "...aborting data file packaging.")
		return()
	endif()
	
	if(IMPORT_STATUS_CODE EQUAL 0)
		message(STATUS "...done, no built-in assets modifications detected.")
		return()
	endif()
	
	message(STATUS "...built-in asset import successful.")
	
	if(${UPLOAD_ASSETS})	
		if(IMPORT_STATUS_CODE EQUAL 1)
			update_data_file_package("${_PREFIX}CompiledData_Master" ${_FOLDER}/Data Raw/DataPackageContents.txt)
		endif()
	endif()
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
