# Find FMOD installation
#
# This module defines
#  FMOD_INCLUDE_DIRS
#  FMOD_LIBRARIES
#  FMOD_FOUND

B3DStartFindPackage(FMOD)

if(WIN32)
	set(FMOD_INSTALL_DIR "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows" CACHE PATH "")
else()
	if(B3D_USE_BUNDLED_LIBRARIES)
		set(FMOD_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/FMOD CACHE PATH "")
	endif()
endif()

B3DPopulateDefaultPackageSearchPaths(FMOD)

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

B3DFindImportedIncludes(FMOD fmod.h)
B3DFindImportedLibrary(FMOD ${FMOD_LIBNAME} STATIC)

B3DEndFindPackage(FMOD ${FMOD_LIBNAME})
