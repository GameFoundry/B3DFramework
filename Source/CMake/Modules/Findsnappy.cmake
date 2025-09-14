# Find Snappy dependency
#
# This module defines
#  snappy_INCLUDE_DIRS
#  snappy_LIBRARIES
#  snappy_FOUND

B3DStartFindPackage(snappy)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(snappy_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/snappy CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(snappy)

B3DFindImportedIncludes(snappy snappy.h)
B3DFindImportedLibrary(snappy snappy STATIC)

B3DEndFindPackage(snappy snappy)
