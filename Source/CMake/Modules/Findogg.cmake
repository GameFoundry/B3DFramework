# Find ogg dependency
#
# This module defines
#  ogg_INCLUDE_DIRS
#  ogg_LIBRARIES
#  ogg_FOUND

B3DStartFindPackage(ogg)

set(ogg_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/libogg)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT ogg_INSTALL_DIR)
	set(ogg_INSTALL_DIR ${ogg_BUNDLED_INSTALL_DIR} CACHE PATH "Path to ogg dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(ogg BUILD_SCRIPT B3DBuildOgg.sh)

B3DPopulateDefaultPackageSearchPaths(ogg)

B3DFindImportedIncludes(ogg ogg/ogg.h)

if(WIN32)
	B3DFindImportedLibraryWithConfigurationNames(ogg ogg STATIC ogg oggd)
else()
	B3DFindImportedLibrary(ogg ogg STATIC)
endif()

B3DEndFindPackage(ogg ogg)
