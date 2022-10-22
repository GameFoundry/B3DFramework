# Find freetype dependency
#
# This module defines
#  freetype_INCLUDE_DIRS
#  freetype_LIBRARIES
#  freetype_FOUND

start_find_package(freetype)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(freetype_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/freetype CACHE PATH "")
endif()
gen_default_lib_search_dirs(freetype)

list(APPEND freetype_INCLUDE_SEARCH_DIRS /usr/local/include/freetype2 /usr/include/freetype2)

find_imported_includes(freetype freetype/freetype.h)
find_imported_library(freetype freetype)

end_find_package(freetype freetype)

