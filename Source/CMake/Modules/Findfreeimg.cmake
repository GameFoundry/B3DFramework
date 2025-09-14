# Find freeimg dependency
#
# This module defines
#  freeimg_INCLUDE_DIRS
#  freeimg_LIBRARIES
#  freeimg_FOUND

B3DStartFindPackage(freeimg)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(freeimg_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/freeimg CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(freeimg)

B3DFindImportedIncludes(freeimg FreeImage.h)
B3DFindImportedLibrary(freeimg freeimage STATIC)

B3DEndFindPackage(freeimg freeimage)

