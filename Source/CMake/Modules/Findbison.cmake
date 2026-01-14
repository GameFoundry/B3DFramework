# Find bison installation
#
# This module defines
#  bison_EXECUTABLE
#  bison_FOUND

if(B3D_USE_BUNDLED_LIBRARIES)
	set(bison_INSTALL_DIRS ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/bison CACHE PATH "")
endif()

message(STATUS "Looking for bison installation...")

find_program(bison_EXECUTABLE NAMES bison bin/bison PATHS ${bison_INSTALL_DIRS} NO_DEFAULT_PATH)
find_program(bison_EXECUTABLE NAMES bison bin/bison PATHS ${bison_INSTALL_DIRS})

if(bison_EXECUTABLE)
	set(bison_FOUND TRUE)
else()
	set(bison_FOUND FALSE)
endif()

if(NOT bison_FOUND)
	if(bison_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find bison installation. Try modifying the bison_INSTALL_DIRS path.")
		return()
	else()
		message(WARNING "Cannot find bison installation. Try modifying the bison_INSTALL_DIRS path.")
	endif()
else()
	message(STATUS "...bison OK.")
endif()

mark_as_advanced(bison_INSTALL_DIRS bison_EXECUTABLE)
