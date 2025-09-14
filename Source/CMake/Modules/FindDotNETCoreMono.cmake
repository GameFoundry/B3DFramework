# Find mono dependency
#
# This module defines
#  DotNETCoreMono_INCLUDE_DIRS
#  DotNETCoreMono_LIBRARIES
#  DotNETCoreMono_FOUND

B3DStartFindPackage(DotNETCoreMono)

set(DotNETCoreMono_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/DotNETCoreMono CACHE PATH "")

B3DPopulateDefaultPackageSearchPaths(DotNETCoreMono)
list(APPEND DotNETCoreMono_INCLUDE_SEARCH_DIRS ${DotNETCoreMono_INSTALL_DIR}/include)

B3DFindImportedIncludes(DotNETCoreMono mono/jit/jit.h)

# Load library as module as .NET Core requires mono to be in a dynamically loaded library, as we need to be able to reload the library on script recompile
B3DFindImportedLibraryWithAlternateBinaryName(DotNETCoreMono coreclr.import MODULE coreclr)

B3DEndFindPackage(DotNETCoreMono coreclr.import)

# Install the managed assemblies
install(
	DIRECTORY ${DotNETCoreMono_INSTALL_DIR}/bin/Assemblies
	DESTINATION bin/
)
