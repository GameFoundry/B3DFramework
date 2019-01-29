# Find mono dependency
#
# This module defines
#  mono_INCLUDE_DIRS
#  mono_LIBRARIES
#  mono_FOUND

start_find_package(mono)

if(USE_BUNDLED_LIBRARIES)
	set(mono_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/mono CACHE PATH "")
endif()

gen_default_lib_search_dirs(mono)
list(APPEND mono_INCLUDE_SEARCH_DIRS ${mono_INSTALL_DIR}/include/mono-2.0 /usr/include/mono-2.0)

find_imported_includes(mono mono/jit/jit.h)
find_imported_library_shared(mono mono-2.0)

end_find_package(mono mono-2.0)
