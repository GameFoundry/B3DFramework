# Find freetype dependency
#
# This module defines
#  freetype_INCLUDE_DIRS
#  freetype_LIBRARIES
#  freetype_FOUND

B3DStartFindPackage(freetype)

B3DGetBundledDependencyFolder(freetype freetype_BUNDLED_INSTALL_DIR)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT freetype_INSTALL_DIR)
	set(freetype_INSTALL_DIR ${freetype_BUNDLED_INSTALL_DIR} CACHE PATH "Path to freetype dependency" FORCE)
endif()
B3DPopulateDefaultPackageSearchPaths(freetype)

list(APPEND freetype_INCLUDE_SEARCH_DIRS /usr/local/include/freetype2 /usr/include/freetype2)

B3DFindImportedIncludes(freetype freetype/freetype.h)
B3DFindImportedLibrary(freetype freetype STATIC)

B3DEndFindPackage(freetype freetype)

