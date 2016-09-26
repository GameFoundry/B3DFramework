# Find FMOD installation
#
# This module defines
#  FMOD_INCLUDE_DIRS
#  FMOD_LIBRARIES_DEBUG
#  FMOD_LIBRARIES_OPTIMIZED
#  FMOD_LIBRARY_DIRS
#  FMOD_FOUND

# TODO: Set default install paths for mac/unix
set(FMOD_INSTALL_DIRS "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows" CACHE PATH "")
set(FMOD_INCLUDE_SEARCH_DIRS "${FMOD_INSTALL_DIRS}/api/lowlevel/inc")
set(FMOD_LIBRARY_SEARCH_DIRS "${FMOD_INSTALL_DIRS}/api/lowlevel/lib")

message(STATUS "Looking for FMOD installation...")
find_path(FMOD_INCLUDE_DIR fmod.h PATHS ${FMOD_INCLUDE_SEARCH_DIRS})

if(BS_64BIT)
	find_library(FMOD_LIBRARY_OPTIMIZED NAMES fmod64_vc libfmod PATHS ${FMOD_LIBRARY_SEARCH_DIRS})
	find_library(FMOD_LIBRARY_DEBUG NAMES fmodL64_vc libfmodL PATHS ${FMOD_LIBRARY_SEARCH_DIRS})
else()
	find_library(FMOD_LIBRARY_OPTIMIZED NAMES fmod_vc libfmod PATHS ${FMOD_LIBRARY_SEARCH_DIRS})
	find_library(FMOD_LIBRARY_DEBUG NAMES fmodL_vc libfmodL PATHS ${FMOD_LIBRARY_SEARCH_DIRS})
endif()

if(FMOD_INCLUDE_DIR AND FMOD_LIBRARY_OPTIMIZED AND FMOD_LIBRARY_DEBUG)
	set(FMOD_FOUND TRUE)
endif()

if(NOT FMOD_FOUND)
	if(FMOD_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find FMOD installation. Try modifying the FMOD_INSTALL_DIRS path.")
	else()
		message(WARNING "Cannot find FMOD installation. Try modifying the FMOD_INSTALL_DIRS path.")
	endif()
else()
	message(STATUS "...FMOD OK.")
endif()

mark_as_advanced(FMOD_INSTALL_DIRS FMOD_INCLUDE_DIR FMOD_LIBRARY_OPTIMIZED FMOD_LIBRARY_DEBUG)
set(FMOD_INCLUDE_DIRS ${FMOD_INCLUDE_DIR})
set(FMOD_LIBRARIES_OPTIMIZED ${FMOD_LIBRARY_OPTIMIZED})
set(FMOD_LIBRARIES_DEBUG ${FMOD_LIBRARY_DEBUG})