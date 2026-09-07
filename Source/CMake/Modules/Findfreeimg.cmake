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

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(freeimg BUILD_SCRIPT B3DBuildFreeImage.sh)

B3DPopulateDefaultPackageSearchPaths(freeimg)

B3DFindImportedIncludes(freeimg FreeImage.h)

# Dynamic library on Windows, static on Linux/macOS (see dependencies.md)
if(WIN32)
	B3DFindImportedLibraryWithConfigurationNames(freeimg freeimage SHARED FreeImage FreeImaged)
else()
	B3DFindImportedLibrary(freeimg freeimage STATIC)
endif()

B3DEndFindPackage(freeimg freeimage)

