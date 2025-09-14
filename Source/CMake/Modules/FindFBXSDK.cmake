# Find FBXSDK dependency
#
# This module defines
#  FBXSDK_INCLUDE_DIRS
#  FBXSDK_LIBRARIES
#  FBXSDK_FOUND

B3DStartFindPackage(FBXSDK)

# Always use bundled version as package is not easily available
set(FBXSDK_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/FBXSDK CACHE PATH "")
B3DPopulateDefaultPackageSearchPaths(FBXSDK)

if(WIN32)
    set(FBXSDK_LIBNAME libfbxsdk-md)
else()
    set(FBXSDK_LIBNAME fbxsdk)
endif()

B3DFindImportedIncludes(FBXSDK fbxsdk.h)
B3DFindImportedLibrary(FBXSDK ${FBXSDK_LIBNAME} STATIC)

B3DEndFindPackage(FBXSDK ${FBXSDK_LIBNAME})

