# Find vorbis dependency
#
# This module defines
#  vorbis_INCLUDE_DIRS
#  vorbis_LIBRARIES
#  vorbis_FOUND

start_find_package(vorbis)

set(vorbis_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/libvorbis CACHE PATH "")
gen_default_lib_search_dirs(vorbis)

if(WIN32)
	set(vorbis_LIB_PREFIX lib)
endif()

find_imported_includes(vorbis vorbis/vorbisenc.h)

if(UNIX)
	find_imported_library(vorbis ${vorbis_LIB_PREFIX}vorbisfile)
	find_imported_library(vorbis ${vorbis_LIB_PREFIX}vorbisenc)
	find_imported_library(vorbis ${vorbis_LIB_PREFIX}vorbis)
else()
	find_imported_library_shared(vorbis ${vorbis_LIB_PREFIX}vorbis)
	find_imported_library_shared(vorbis ${vorbis_LIB_PREFIX}vorbisfile)
endif()

install_dependency_binaries(vorbis)

end_find_package(vorbis ${vorbis_LIB_PREFIX}vorbis)
