# Find MoltenVK installation
#
# This module defines
#  MoltenVK_INCLUDE_DIRS
#  MoltenVK_LIBRARIES
#  MoltenVK_FOUND

B3DStartFindPackage(MoltenVK)

set(MoltenVK_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/MoltenVK CACHE PATH "")
B3DPopulateDefaultPackageSearchPaths(MoltenVK)

B3DFindImportedIncludes(MoltenVK MoltenVK/mvk_vulkan.h)
B3DFindImportedLibrary(MoltenVK MoltenVK STATIC)

B3DEndFindPackage(MoltenVK MoltenVK)
