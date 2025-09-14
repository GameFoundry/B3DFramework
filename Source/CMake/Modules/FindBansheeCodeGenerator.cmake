# Find BansheeCodeGenerator tool dependency
#
# This module defines
#  BansheeCodeGenerator_EXECUTABLE_PATH
#  BansheeCodeGenerator_FOUND

set(BansheeCodeGenerator_INSTALL_DIRS ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/tools/BansheeCodeGenerator/bin CACHE PATH "")

message(STATUS "Looking for BansheeCodeGenerator installation...")
find_program(BansheeCodeGenerator_EXECUTABLE NAMES BansheeCodeGenerator PATHS ${BansheeCodeGenerator_INSTALL_DIRS})

if(BansheeCodeGenerator_EXECUTABLE)
	set(BansheeCodeGenerator_FOUND TRUE)
endif()

if(NOT BansheeCodeGenerator_FOUND)
	if(BansheeCodeGenerator_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find BansheeCodeGenerator installation. Try modifying the BansheeCodeGenerator_INSTALL_DIRS path.")
	else()
		message(WARNING "Cannot find BansheeCodeGenerator installation. Try modifying the BansheeCodeGenerator_INSTALL_DIRS path.")
	endif()
else()
	message(STATUS "...BansheeCodeGenerator OK.")
endif()

mark_as_advanced(
	BansheeCodeGenerator_INSTALL_DIRS
	BansheeCodeGenerator_EXECUTABLE)

set(BansheeCodeGenerator_EXECUTABLE_PATH ${BansheeCodeGenerator_EXECUTABLE})
