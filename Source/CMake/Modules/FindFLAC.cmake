# Find FLAC dependency
#
# This module defines
#  FLAC_INCLUDE_DIRS
#  FLAC_LIBRARIES
#  FLAC_FOUND

B3DStartFindPackage(FLAC)

set(FLAC_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/libFLAC)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT FLAC_INSTALL_DIR)
	set(FLAC_INSTALL_DIR ${FLAC_BUNDLED_INSTALL_DIR} CACHE PATH "Path to FLAC dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(FLAC BUILD_SCRIPT B3DBuildFLAC.sh)

B3DPopulateDefaultPackageSearchPaths(FLAC)

B3DFindImportedIncludes(FLAC FLAC/all.h)

if(WIN32)
	B3DFindImportedLibraryWithConfigurationNames(FLAC FLAC STATIC FLAC FLACd)
else()
	B3DFindImportedLibrary(FLAC FLAC STATIC)
endif()

B3DEndFindPackage(FLAC FLAC)
