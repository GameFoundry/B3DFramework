# Find RakNet dependency
#
# This module defines
#  RakNet_INCLUDE_DIRS
#  RakNet_LIBRARIES
#  RakNet_FOUND

start_find_package(RakNet)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(RakNet_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/RakNet CACHE PATH "")
endif()

gen_default_lib_search_dirs(RakNet)
find_imported_includes(RakNet RakNet/RakPeer.h)
find_imported_library_shared(RakNet RakNetLibStatic)

end_find_package(RakNet RakNetLibStatic)
