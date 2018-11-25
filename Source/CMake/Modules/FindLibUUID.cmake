# This module defines:
#  LibUUID_INCLUDE_DIRS        Location of libuuid headers
#  LibUUID_LIBRARIES           List of libraries to use libuuid
#  LibUUID_FOUND               True if libuuid was found

find_path(LibUUID_INCLUDE_DIR uuid/uuid.h)

if(APPLE)
	find_library(LibUUID_LIBRARY NAMES libuuid.a uuid)
else()
	find_library(LibUUID_LIBRARY NAMES uuid)
endif()

if(LibUUID_INCLUDE_DIR AND LibUUID_LIBRARY)
    set(LibUUID_FOUND TRUE)
endif()

message(STATUS "Looking for LibUUID...")

if(NOT LibUUID_FOUND)
    if(LibUUID_FIND_REQUIRED)
        message(FATAL_ERROR "Cannot find LibUUID.")
    elseif(NOT LibUUID_FIND_QUIETLY)
        message(WARNING "Cannot find LibUUID.")
    endif()
else()
    message(STATUS "...LibUUID OK.")
endif()

if(LibUUID_FOUND)
    add_library(LibUUID STATIC IMPORTED)
    set_target_properties(LibUUID PROPERTIES IMPORTED_LOCATION "${LibUUID_LIBRARY}")
    set_target_properties(LibUUID PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LibUUID_INCLUDE_DIR}")
endif()

set(LibUUID_INCLUDE_DIRS ${LibUUID_INCLUDE_DIR})
set(LibUUID_LIBRARIES LibUUID)

mark_as_advanced(
        LibUUID_INCLUDE_DIR
        LibUUID_LIBRARY
)
