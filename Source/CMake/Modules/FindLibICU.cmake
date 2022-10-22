# Find LibICU dependency
#
# This module defines
#  LibICU_INCLUDE_DIRS
#  LibICU_LIBRARIES
#  LibICU_FOUND

start_find_package(LibICU)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(LibICU_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/libICU CACHE PATH "")
endif()
gen_default_lib_search_dirs(LibICU)

find_imported_includes(LibICU unicode/utypes.h)
find_imported_library(LibICU icudata)
find_imported_library(LibICU icuuc)

end_find_package(LibICU icudata)
