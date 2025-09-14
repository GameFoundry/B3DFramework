# Find FLAC dependency
#
# This module defines
#  FLAC_INCLUDE_DIRS
#  FLAC_LIBRARIES
#  FLAC_FOUND

B3DStartFindPackage(FLAC)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(FLAC_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/libFLAC CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(FLAC)

if(WIN32)
	set(FLAC_LIBNAME libFLAC)
else()
	set(FLAC_LIBNAME FLAC)
endif()

B3DFindImportedIncludes(FLAC FLAC/all.h)

if(UNIX)
	B3DFindImportedLibrary(FLAC ${FLAC_LIBNAME} STATIC)
else()
	B3DFindImportedLibraryWithAlternateBinaryName(FLAC ${FLAC_LIBNAME} SHARED libFLAC_dynamic)
endif()

B3DEndFindPackage(FLAC ${FLAC_LIBNAME})
