# Find ogg dependency
#
# This module defines
#  ogg_INCLUDE_DIRS
#  ogg_LIBRARIES
#  ogg_FOUND

start_find_package(ogg)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(ogg_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/libogg CACHE PATH "")
endif()
gen_default_lib_search_dirs(ogg)

if(WIN32)
	set(ogg_LIBNAME libogg)
else()
	set(ogg_LIBNAME ogg)
endif()

find_imported_includes(ogg ogg/ogg.h)
find_imported_library(ogg ${ogg_LIBNAME})

end_find_package(ogg ${ogg_LIBNAME})
