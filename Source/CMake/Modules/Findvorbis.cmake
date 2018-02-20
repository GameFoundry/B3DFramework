# Find vorbis dependency
#
# This module defines
#  vorbis_INCLUDE_DIRS
#  vorbis_LIBRARIES
#  vorbis_FOUND

start_find_package(vorbis)

set(vorbis_INSTALL_DIR ${PROJECT_SOURCE_DIR}/../Dependencies/libvorbis CACHE PATH "")
gen_default_lib_search_dirs(vorbis)

if(WIN32)
	set(vorbis_LIB_PREFIX lib)
endif()

find_imported_includes(vorbis vorbis/vorbisenc.h)
find_imported_library_shared(vorbis ${vorbis_LIB_PREFIX}vorbis)
find_imported_library_shared(vorbis ${vorbis_LIB_PREFIX}vorbisfile)

if(APPLE)
	find_imported_library_shared(vorbis ${vorbis_LIB_PREFIX}vorbisenc)
endif()

install_dependency_binaries(vorbis)

end_find_package(vorbis ${vorbis_LIB_PREFIX}vorbis)
