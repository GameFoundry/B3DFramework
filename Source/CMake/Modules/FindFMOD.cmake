# Find FMOD installation
#
# This module defines
#  FMOD_INCLUDE_DIRS
#  FMOD_LIBRARIES
#  FMOD_FOUND

start_find_package(FMOD)

if(WIN32)
	set(FMOD_INSTALL_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows" CACHE PATH "")
else()
	if(B3D_USE_BUNDLED_LIBRARIES)
		set(FMOD_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/FMOD CACHE PATH "")
	endif()
endif()

gen_default_lib_search_dirs(FMOD)

list(APPEND FMOD_INCLUDE_SEARCH_DIRS ${FMOD_INSTALL_DIR}/api/lowlevel/inc)

if(WIN32)
	set(FMOD_LIBRARY_SEARCH_PATH ${FMOD_INSTALL_DIR}/api/lowlevel/lib)

	if(B3D_IS_64BIT)
		set(FMOD_LIBNAME fmod64_vc)
	else()
		set(FMOD_LIBNAME fmod_vc)
	endif()
else()
	set(FMOD_LIBNAME fmod)
	if(B3D_IS_64BIT)
		set(FMOD_LIBRARY_SEARCH_PATH ${FMOD_INSTALL_DIR}/api/lowlevel/lib/x86_64)
	else()
		set(FMOD_LIBRARY_SEARCH_PATH ${FMOD_INSTALL_DIR}/api/lowlevel/lib/x86)
	endif()
endif()

list(APPEND FMOD_LIBRARY_RELEASE_SEARCH_DIRS ${FMOD_LIBRARY_SEARCH_PATH})
list(APPEND FMOD_LIBRARY_DEBUG_SEARCH_DIRS ${FMOD_LIBRARY_SEARCH_PATH})

find_imported_includes(FMOD fmod.h)
find_imported_library(FMOD ${FMOD_LIBNAME})

end_find_package(FMOD ${FMOD_LIBNAME})
