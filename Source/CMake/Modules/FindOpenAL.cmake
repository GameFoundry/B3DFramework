# Find OpenAL dependency
#
# This module defines
#  OpenAL_INCLUDE_DIRS
#  OpenAL_LIBRARIES
#  OpenAL_FOUND

B3DStartFindPackage(OpenAL)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(OpenAL_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/OpenAL CACHE PATH "")
endif()
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
