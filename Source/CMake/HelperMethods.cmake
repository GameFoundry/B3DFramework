function(add_prefix var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
endfunction()

function(add_engine_dependencies target_name)
	if(RENDER_API_MODULE MATCHES "DirectX 11")
		add_dependencies(${target_name} bsfD3D11RenderAPI)
	elseif(RENDER_API_MODULE MATCHES "Vulkan")
		add_dependencies(${target_name} bsfVulkanRenderAPI)
	else()
		add_dependencies(${target_name} bsfGLRenderAPI)
	endif()

	if(AUDIO_MODULE MATCHES "FMOD")
		add_dependencies(${target_name} bsfFMOD)
	else() # Default to OpenAudio
		add_dependencies(${target_name} bsfOpenAudio)
	endif()
	
	add_dependencies(${target_name} bsfSL bsfPhysX bsfRenderBeast)
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
		set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${RELEASE_NAME}")
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
			set(DESTINATION_DIR lib)
		endif()
		
		install(
			FILES ${SRC_RELEASE}
			DESTINATION ${DESTINATION_DIR}
			CONFIGURATIONS Release OptimizedDebug
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
endfunction()

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

	set(BINARY_DEPENDENCIES_URL http://data.banshee3d.com/${DEP_PREFIX}Dependencies_${DEP_TYPE}_Master_${DEP_VERSION}.zip)
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
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/bin ${PROJECT_SOURCE_DIR}/bin)	
	
	# Copy static libraries, headers and tools
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEP_FOLDER})	
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/Dependencies ${DEP_FOLDER})
	
	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
endfunction()

function(check_and_update_binary_deps DEP_PREFIX DEP_FOLDER DEP_VERSION)
	if(NOT APPLE)
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
	endif()
endfunction()

function(update_builtin_assets ASSET_PREFIX ASSET_FOLDER ASSET_VERSION)
	# Clean and create a temporary folder
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
	execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/Temp)	
	
	set(ASSET_DEPENDENCIES_URL http://data.banshee3d.com/${ASSET_PREFIX}Data_Master_${ASSET_VERSION}.zip)
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
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Temp/Data ${ASSET_FOLDER})
	
	# Make sure timestamp modify date/times are newer (avoids triggering reimport)
	execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${ASSET_FOLDER}/Timestamp.asset )
	
	# Make sure resource manifests get rebuilt
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${ASSET_FOLDER}/ResourceManifest.asset)
	
	# Clean up
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/Temp)	
endfunction()

function(check_and_update_builtin_assets ASSET_PREFIX ASSET_FOLDER ASSET_VERSION)
	set(BUILTIN_ASSETS_VERSION_FILE ${ASSET_FOLDER}/.version)
	if(NOT EXISTS ${BUILTIN_ASSETS_VERSION_FILE})
		message(STATUS "Builtin assets for '${ASSET_PREFIX}' are missing. Downloading package...")
		update_builtin_assets(${ASSET_PREFIX} ${ASSET_FOLDER} ${ASSET_VERSION})	
	else()
		file (STRINGS ${BUILTIN_ASSETS_VERSION_FILE} CURRENT_BUILTIN_ASSET_VERSION)
		if(${ASSET_VERSION} GREATER ${CURRENT_BUILTIN_ASSET_VERSION})
			message(STATUS "Your builtin asset package for '${ASSET_PREFIX}' is out of date. Downloading latest package...")
			update_builtin_assets(${ASSET_PREFIX} ${ASSET_FOLDER} ${ASSET_VERSION})	
		endif()
	endif()
endfunction()