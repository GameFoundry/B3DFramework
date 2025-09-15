# Find RenderDoc
#
# This module defines
#  RenderDoc_INCLUDE_DIRS
#  RenderDoc_LIBRARIES
#  RenderDoc_FOUND

B3DStartFindPackage(RenderDoc)

set(RenderDoc_INSTALL_DIR ${B3D_DEPENDENCY_DIRECTORY}/RenderDoc CACHE PATH "")
B3DPopulateDefaultPackageSearchPaths(RenderDoc)

B3DFindImportedIncludes(RenderDoc RenderDoc/renderdoc_app.h)
B3DFindImportedLibrary(RenderDoc renderdoc MODULE)

B3DEndFindPackage(RenderDoc renderdoc)