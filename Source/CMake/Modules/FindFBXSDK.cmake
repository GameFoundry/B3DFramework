# Find FBXSDK dependency
#
# This module defines
#  FBXSDK_INCLUDE_DIRS
#  FBXSDK_LIBRARIES
#  FBXSDK_FOUND

B3DStartFindPackage(FBXSDK)

set(FBXSDK_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/FBXSDK)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT FBXSDK_INSTALL_DIR)
	set(FBXSDK_INSTALL_DIR ${FBXSDK_BUNDLED_INSTALL_DIR} CACHE PATH "Path to FBXSDK dependency" FORCE)
endif()
B3DPopulateDefaultPackageSearchPaths(FBXSDK)

if(WIN32)
    set(FBXSDK_LIBNAME libfbxsdk-md)
else()
    set(FBXSDK_LIBNAME fbxsdk)
endif()

B3DFindImportedIncludes(FBXSDK fbxsdk.h)
B3DFindImportedLibrary(FBXSDK ${FBXSDK_LIBNAME} STATIC)

B3DEndFindPackage(FBXSDK ${FBXSDK_LIBNAME})

# The static FBX library requires these system libraries on macOS
if(APPLE)
	list(APPEND FBXSDK_LIBRARIES xml2 z iconv)
endif()

