# Find FLAC dependency
#
# This module defines
#  FLAC_INCLUDE_DIRS
#  FLAC_LIBRARIES
#  FLAC_FOUND

start_find_package(FLAC)

set(FLAC_INSTALL_DIR ${PROJECT_SOURCE_DIR}/../Dependencies/libFLAC CACHE PATH "")
gen_default_lib_search_dirs(FLAC)

if(WIN32)
	set(FLAC_LIBNAME libFLAC)
else()
	set(FLAC_LIBNAME FLAC)
endif()

find_imported_includes(FLAC FLAC/all.h)
find_imported_library_shared(FLAC ${FLAC_LIBNAME})

install_dependency_binaries(FLAC)

end_find_package(FLAC ${FLAC_LIBNAME})
