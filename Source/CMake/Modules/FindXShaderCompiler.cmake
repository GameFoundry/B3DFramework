# Find XShaderCompiler dependency
#
# This module defines
#  XShaderCompiler_INCLUDE_DIRS
#  XShaderCompiler_LIBRARIES
#  XShaderCompiler_FOUND

B3DStartFindPackage(XShaderCompiler)

set(XShaderCompiler_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/XShaderCompiler CACHE PATH "")
B3DPopulateDefaultPackageSearchPaths(XShaderCompiler)

B3DFindImportedIncludes(XShaderCompiler Xsc/Xsc.h)
B3DFindImportedLibrary(XShaderCompiler xsc_core STATIC)

B3DEndFindPackage(XShaderCompiler xsc_core)
