# Find ogg dependency
#
# This module defines
#  ogg_INCLUDE_DIRS
#  ogg_LIBRARIES
#  ogg_FOUND

B3DStartFindPackage(ogg)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(ogg_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/libogg CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(ogg)

if(WIN32)
	set(ogg_LIBNAME libogg)
else()
	set(ogg_LIBNAME ogg)
endif()

B3DFindImportedIncludes(ogg ogg/ogg.h)
B3DFindImportedLibrary(ogg ${ogg_LIBNAME} STATIC)

B3DEndFindPackage(ogg ${ogg_LIBNAME})
