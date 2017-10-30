# Find BansheeSBGen tool dependency
#
# This module defines
#  BansheeSBGen_EXECUTABLE_PATH
#  BansheeSBGen_FOUND

set(BansheeSBGen_INSTALL_DIRS ${PROJECT_SOURCE_DIR}/../Dependencies/tools/BansheeSBGen/bin CACHE PATH "")

message(STATUS "Looking for BansheeSBGen installation...")
find_program(BansheeSBGen_EXECUTABLE NAMES BansheeSBGen PATHS ${BansheeSBGen_INSTALL_DIRS})

if(BansheeSBGen_EXECUTABLE)
	set(BansheeSBGen_FOUND TRUE)
endif()

if(NOT BansheeSBGen_FOUND)
	if(BansheeSBGen_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find BansheeSBGen installation. Try modifying the BansheeSBGen_INSTALL_DIRS path.")
	else()
		message(WARNING "Cannot find BansheeSBGen installation. Try modifying the BansheeSBGen_INSTALL_DIRS path.")
	endif()
else()
	message(STATUS "...BansheeSBGen OK.")
endif()

mark_as_advanced(
	BansheeSBGen_INSTALL_DIRS 
	BansheeSBGen_EXECUTABLE)

set(BansheeSBGen_EXECUTABLE_PATH ${BansheeSBGen_EXECUTABLE})