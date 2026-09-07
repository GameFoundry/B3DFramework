# Find OpenAL dependency
#
# This module defines
#  OpenAL_INCLUDE_DIRS
#  OpenAL_LIBRARIES
#  OpenAL_FOUND

B3DStartFindPackage(OpenAL)

set(OpenAL_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/OpenAL)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT OpenAL_INSTALL_DIR)
	set(OpenAL_INSTALL_DIR ${OpenAL_BUNDLED_INSTALL_DIR} CACHE PATH "Path to OpenAL dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(OpenAL BUILD_SCRIPT B3DBuildOpenAL.sh)

B3DPopulateDefaultPackageSearchPaths(OpenAL)

if(WIN32)
	set(OpenAL_LIBNAME OpenAL32)
else()
	set(OpenAL_LIBNAME openal)
endif()

B3DFindImportedIncludes(OpenAL AL/al.h)

if(APPLE)
	B3DFindImportedLibrary(OpenAL ${OpenAL_LIBNAME} STATIC)
else()
	B3DFindImportedLibrary(OpenAL ${OpenAL_LIBNAME} SHARED)
endif()

B3DEndFindPackage(OpenAL ${OpenAL_LIBNAME})
