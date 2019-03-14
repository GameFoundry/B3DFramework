# Find mono dependency
#
# This module defines
#  mono_INCLUDE_DIRS
#  mono_LIBRARIES
#  mono_FOUND

start_find_package(mono)

set(mono_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/mono CACHE PATH "")

gen_default_lib_search_dirs(mono)
list(APPEND mono_INCLUDE_SEARCH_DIRS ${mono_INSTALL_DIR}/include/mono-2.0 /usr/include/mono-2.0)

find_imported_includes(mono mono/jit/jit.h)
find_imported_library_shared(mono mono-2.0)

if(WIN32)
	# .dll has a different name than .lib, so we must register it separately
	install_dependency_dll(mono ${mono_INSTALL_DIR} mono-2.0-sgen)
endif()
	
install_dependency_binaries(mono)

end_find_package(mono mono-2.0)

# Install the managed libraries and config files required by Mono
install(
	DIRECTORY ${mono_INSTALL_DIR}/bin/Mono
	DESTINATION bin/
)