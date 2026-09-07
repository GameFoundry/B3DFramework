# Find XShaderCompiler dependency
#
# This module defines
#  XShaderCompiler_INCLUDE_DIRS
#  XShaderCompiler_LIBRARIES
#  XShaderCompiler_FOUND

B3DStartFindPackage(XShaderCompiler)

set(XShaderCompiler_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/XShaderCompiler)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT XShaderCompiler_INSTALL_DIR)
	set(XShaderCompiler_INSTALL_DIR ${XShaderCompiler_BUNDLED_INSTALL_DIR} CACHE PATH "Path to XShaderCompiler dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(XShaderCompiler BUILD_SCRIPT B3DBuildShaderCompiler.sh)

B3DPopulateDefaultPackageSearchPaths(XShaderCompiler)

B3DFindImportedIncludes(XShaderCompiler Xsc/Xsc.h)
B3DFindImportedLibrary(XShaderCompiler xsc_core SHARED)

B3DEndFindPackage(XShaderCompiler xsc_core)

if(TARGET XShaderCompiler::xsc_core)
	set_property(TARGET XShaderCompiler::xsc_core APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS XSC_SHARED_LIB)
endif()
