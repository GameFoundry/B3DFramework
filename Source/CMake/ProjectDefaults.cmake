set (BSF_SOURCE_DIR ${BSF_DIRECTORY}/Source)
set (APP_ROOT_DIR ${BSF_DIRECTORY})
set (BS_IS_BANSHEE3D 0)

include(${BSF_SOURCE_DIR}/CMake/Properties.cmake)
include(${BSF_SOURCE_DIR}/CMake/FindPackageOrBuild.cmake)
include(${BSF_SOURCE_DIR}/CMake/HelperMethods.cmake)

add_subdirectory(${BSF_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/bsf)