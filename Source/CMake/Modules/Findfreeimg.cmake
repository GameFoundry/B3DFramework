# Find freeimg dependency
#
# This module defines
#  freeimg_INCLUDE_DIRS
#  freeimg_LIBRARIES
#  freeimg_FOUND

B3DStartFindPackage(freeimg)

set(freeimg_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/freeimg)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT freeimg_INSTALL_DIR)
	set(freeimg_INSTALL_DIR ${freeimg_BUNDLED_INSTALL_DIR} CACHE PATH "Path to freeimg dependency" FORCE)
endif()
B3DPopulateDefaultPackageSearchPaths(freeimg)

B3DFindImportedIncludes(freeimg FreeImage.h)
B3DFindImportedLibraryWithConfigurationNames(freeimg freeimage SHARED FreeImage FreeImaged)

B3DEndFindPackage(freeimg freeimage)

