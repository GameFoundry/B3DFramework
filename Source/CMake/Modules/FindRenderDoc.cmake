# Find RenderDoc
#
# This module defines
#  RenderDoc_INCLUDE_DIRS
#  RenderDoc_LIBRARIES
#  RenderDoc_FOUND

B3DStartFindPackage(RenderDoc)

set(RenderDoc_BUNDLED_INSTALL_DIR ${B3D_DEPENDENCY_DIRECTORY}/RenderDoc)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT RenderDoc_INSTALL_DIR)
	set(RenderDoc_INSTALL_DIR ${RenderDoc_BUNDLED_INSTALL_DIR} CACHE PATH "Path to RenderDoc dependency" FORCE)
endif()

# Ensure the bundled copy is up to date
B3DEnsureBundledDependency(RenderDoc)

B3DPopulateDefaultPackageSearchPaths(RenderDoc)

B3DFindImportedIncludes(RenderDoc RenderDoc/renderdoc_app.h)
B3DFindImportedLibrary(RenderDoc renderdoc MODULE)

B3DEndFindPackage(RenderDoc renderdoc)
