# Find SPIRVCross
#
# This module defines
#  SPIRVCross_INCLUDE_DIRS
#  SPIRVCross_LIBRARIES
#  SPIRVCross_FOUND

start_find_package(SPIRVCross)

if(USE_BUNDLED_LIBRARIES)
	set(SPIRVCross_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/SPIRVCross CACHE PATH "")
endif()

gen_default_lib_search_dirs(SPIRVCross)

find_imported_includes(SPIRVCross spirv_cross/spirv_cross.hpp)
find_imported_library(SPIRVCross spirv-cross-msl)
find_imported_library(SPIRVCross spirv-cross-glsl)
find_imported_library(SPIRVCross spirv-cross-core)

end_find_package(SPIRVCross spirv-cross-msl)