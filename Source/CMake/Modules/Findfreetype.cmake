# Find freetype dependency
#
# This module defines
#  freetype_INCLUDE_DIRS
#  freetype_LIBRARIES
#  freetype_FOUND

B3DStartFindPackage(freetype)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(freetype_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/freetype CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(freetype)

list(APPEND freetype_INCLUDE_SEARCH_DIRS /usr/local/include/freetype2 /usr/include/freetype2)

B3DFindImportedIncludes(freetype freetype/freetype.h)
B3DFindImportedLibrary(freetype freetype STATIC)

B3DEndFindPackage(freetype freetype)

