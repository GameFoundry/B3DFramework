# Find vorbis dependency
#
# This module defines
#  vorbis_INCLUDE_DIRS
#  vorbis_LIBRARIES
#  vorbis_FOUND

B3DStartFindPackage(vorbis)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(vorbis_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/libvorbis CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(vorbis)

if(WIN32)
	set(vorbis_LIB_PREFIX lib)
endif()

B3DFindImportedIncludes(vorbis vorbis/vorbisenc.h)

if(UNIX)
	B3DFindImportedLibrary(vorbis ${vorbis_LIB_PREFIX}vorbisfile STATIC)
	B3DFindImportedLibrary(vorbis ${vorbis_LIB_PREFIX}vorbisenc STATIC)
	B3DFindImportedLibrary(vorbis ${vorbis_LIB_PREFIX}vorbis STATIC)
else()
	B3DFindImportedLibrary(vorbis ${vorbis_LIB_PREFIX}vorbis SHARED)
	B3DFindImportedLibrary(vorbis ${vorbis_LIB_PREFIX}vorbisfile SHARED)
endif()

B3DEndFindPackage(vorbis ${vorbis_LIB_PREFIX}vorbis)
