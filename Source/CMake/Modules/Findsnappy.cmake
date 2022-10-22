# Find Snappy dependency
#
# This module defines
#  snappy_INCLUDE_DIRS
#  snappy_LIBRARIES
#  snappy_FOUND

start_find_package(snappy)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(snappy_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/snappy CACHE PATH "")
endif()
gen_default_lib_search_dirs(snappy)

find_imported_includes(snappy snappy.h)
find_imported_library(snappy snappy)

end_find_package(snappy snappy)
