cmake_minimum_required (VERSION 3.12.0)
project (bsf)

set (BSF_SOURCE_DIR ${PROJECT_SOURCE_DIR}/Source)
set (APP_ROOT_DIR ${PROJECT_SOURCE_DIR})
set (BS_IS_BANSHEE3D 0)

# Default install dir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	if(LINUX)
		set(CMAKE_INSTALL_PREFIX /usr/local/opt/bsf CACHE PATH "..." FORCE)
	endif()
endif()

include(Source/CMake/Properties.cmake)
include(Source/CMake/FindPackageOrBuild.cmake)
include(Source/CMake/HelperMethods.cmake)

add_subdirectory(Source)
