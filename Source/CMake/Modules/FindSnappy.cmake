# Find Snappy dependency
#
# This module defines
#  Snappy_INCLUDE_DIRS
#  Snappy_LIBRARIES
#  Snappy_FOUND

set(Snappy_INSTALL_DIR ${PROJECT_SOURCE_DIR}/../Dependencies/Snappy CACHE PATH "")
set(Snappy_INCLUDE_SEARCH_DIRS "${Snappy_INSTALL_DIR}/include")

if(BS_64BIT)
	list(APPEND Snappy_LIBRARY_RELEASE_SEARCH_DIRS "${Snappy_INSTALL_DIR}/lib/x64/Release")
	list(APPEND Snappy_LIBRARY_DEBUG_SEARCH_DIRS "${Snappy_INSTALL_DIR}/lib/x64/Debug")
else()
	list(APPEND Snappy_LIBRARY_RELEASE_SEARCH_DIRS "${Snappy_INSTALL_DIR}/lib/x86/Release")
	list(APPEND Snappy_LIBRARY_DEBUG_SEARCH_DIRS "${Snappy_INSTALL_DIR}/lib/x86/Debug")
endif()

message(STATUS "Looking for Snappy installation...")
	
find_path(Snappy_INCLUDE_DIR snappy.h PATHS ${Snappy_INCLUDE_SEARCH_DIRS})	
	
if(Snappy_INCLUDE_DIR)
	set(Snappy_FOUND TRUE)
else()
	set(Snappy_FOUND FALSE)
endif()	
	
find_imported_library(Snappy Snappy)

if(NOT Snappy_FOUND)
	if(Snappy_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find Snappy installation. Try modifying the Snappy_INSTALL_DIR path.")
	elseif(NOT Snappy_FIND_QUIETLY)
		message(WARNING "Cannot find Snappy installation. Try modifying the Snappy_INSTALL_DIR path.")
	endif()
else()
	message(STATUS "...Snappy OK.")
endif()

mark_as_advanced(Snappy_INSTALL_DIR Snappy_INCLUDE_DIR)

set(Snappy_INCLUDE_DIRS ${Snappy_INCLUDE_DIR})
set(Snappy_LIBRARIES Snappy)