# Find bsf import tool dependency
#
# This module defines
#  bsfImportTool_EXECUTABLE_PATH
#  bsfImportTool_FOUND

set(bsfImportTool_INSTALL_DIRS ${BSF_SOURCE_DIR}/../Dependencies/tools/bsfImportTool/bin CACHE PATH "")

if(NOT bsfImportTool_FIND_QUIET)
	message(STATUS "Looking for bsfImportTool installation...")
endif()

find_program(bsfImportTool_EXECUTABLE NAMES bsfImportTool PATHS ${bsfImportTool_INSTALL_DIRS})

if(bsfImportTool_EXECUTABLE)
	set(bsfImportTool_FOUND TRUE)
endif()

if(NOT bsfImportTool_FIND_QUIET)
	if(NOT bsfImportTool_FOUND)
		if(bsfImportTool_FIND_REQUIRED)
			message(FATAL_ERROR "Cannot find bsfImportTool installation. Try modifying the bsfImportTool_INSTALL_DIRS path.")
		else()
			message(WARNING "Cannot find bsfImportTool installation. Try modifying the bsfImportTool_INSTALL_DIRS path.")
		endif()
	else()
		message(STATUS "...bsfImportTool OK.")
	endif()
endif()

mark_as_advanced(
	bsfImportTool_INSTALL_DIRS 
	bsfImportTool_EXECUTABLE)

set(bsfImportTool_EXECUTABLE_PATH ${bsfImportTool_EXECUTABLE})