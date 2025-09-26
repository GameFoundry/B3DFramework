# Find bsf dependency
#
# This module defines
#  bsf_INCLUDE_DIRS
#  bsf_LIBRARIES
#  bsf_FOUND

message(STATUS "Looking for bsf installation...")
if(NOT bsf_INSTALL_DIR)
	set(bsf_INSTALL_DIR ${PROJECT_SOURCE_DIR}/Dependencies/bsf CACHE PATH "")
endif()

set(bsf_INCLUDE_SEARCH_DIRS "${bsf_INSTALL_DIR}/include")
set(INCLUDE_FILES "Core/B3DPrerequisites.h")

find_path(bsf_INCLUDE_DIR NAMES NAMES ${INCLUDE_FILES} PATHS ${bsf_INCLUDE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_path(bsf_INCLUDE_DIR NAMES NAMES ${INCLUDE_FILES} PATHS ${bsf_INCLUDE_SEARCH_DIRS})

if(bsf_INCLUDE_DIR)
	set(bsf_FOUND TRUE)
else()
	message(STATUS "...Cannot find include file \"${INCLUDE_FILES}\" at path ${bsf_INCLUDE_SEARCH_DIRS}")
	set(bsf_FOUND FALSE)
endif()

# Add configuration specific search directories
list(APPEND bsf_LIBRARY_RELEASE_SEARCH_DIRS "${bsf_INSTALL_DIR}/lib/Release")
list(APPEND bsf_LIBRARY_DEBUG_SEARCH_DIRS "${bsf_INSTALL_DIR}/lib/Debug")

# Add search directories with no specified configuration
list(APPEND bsf_LIBRARY_RELEASE_SEARCH_DIRS "${bsf_INSTALL_DIR}/lib")
list(APPEND bsf_LIBRARY_DEBUG_SEARCH_DIRS "${bsf_INSTALL_DIR}/lib")

find_library(bsf_LIBRARY_RELEASE NAMES bsf PATHS ${bsf_LIBRARY_RELEASE_SEARCH_DIRS} NO_DEFAULT_PATH)
find_library(bsf_LIBRARY_RELEASE NAMES bsf PATHS ${bsf_LIBRARY_RELEASE_SEARCH_DIRS})

find_library(bsf_LIBRARY_DEBUG NAMES bsf PATHS ${bsf_LIBRARY_DEBUG_SEARCH_DIRS} NO_DEFAULT_PATH)
find_library(bsf_LIBRARY_DEBUG NAMES bsf PATHS ${bsf_LIBRARY_DEBUG_SEARCH_DIRS})

if(bsf_LIBRARY_RELEASE)
	if(NOT WIN32)
		add_library(bsf SHARED IMPORTED)
	else()
		add_library(bsf STATIC IMPORTED)
	endif()
	
	if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
		set_target_properties(bsf PROPERTIES IMPORTED_LOCATION_RELEASE "${bsf_LIBRARY_RELEASE}")
		set_target_properties(bsf PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${bsf_LIBRARY_RELEASE}")
		set_target_properties(bsf PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${bsf_LIBRARY_RELEASE}")
	
		if(bsf_LIBRARY_DEBUG)
			set_target_properties(bsf PROPERTIES IMPORTED_LOCATION_DEBUG "${bsf_LIBRARY_DEBUG}")
		else()
			set_target_properties(bsf PROPERTIES IMPORTED_LOCATION_DEBUG "${bsf_LIBRARY_RELEASE}")
		endif()
	else()
		set_target_properties(bsf PROPERTIES IMPORTED_LOCATION "${bsf_LIBRARY_RELEASE}")
	endif()
	
	set(bsf_LIBRARIES bsf)	
else()
	set(bsf_FOUND FALSE)
endif()

mark_as_advanced(bsf_LIBRARY_RELEASE)
mark_as_advanced(bsf_LIBRARY_DEBUG)

if(NOT bsf_FOUND)
	if(bsf_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find bsf installation. Try modifying the bsf_INSTALL_DIR path.")
	elseif(NOT bsf_FIND_QUIETLY)
		message(WARNING "Cannot find bsf installation. Try modifying the bsf_INSTALL_DIR path.")
	endif()
else()
	list(APPEND INCLUDE_DIRS "${bsf_INCLUDE_DIR}/Utility")
	list(APPEND INCLUDE_DIRS "${bsf_INCLUDE_DIR}/Core")

	set_target_properties(bsf PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIRS}")
	mark_as_advanced(bsf_INSTALL_DIR)
	message(STATUS "...bsf OK.")
endif()

mark_as_advanced(bsf_INCLUDE_DIR)
set(bsf_INCLUDE_DIRS ${bsf_INCLUDE_DIR})
