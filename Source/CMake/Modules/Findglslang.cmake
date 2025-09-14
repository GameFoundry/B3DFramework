# Find glslang
#
# This module defines
#  glslang_INCLUDE_DIRS
#  glslang_LIBRARIES
#  glslang_FOUND

B3DStartFindPackage(glslang)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(glslang_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/glslang CACHE PATH "")
endif()
B3DPopulateDefaultPackageSearchPaths(glslang)

if(WIN32)
	set(glslang_DEBUG_SUFFIX "d")
else()
	set(glslang_DEBUG_SUFFIX "")
endif()

B3DFindImportedIncludes(glslang glslang/Public/ShaderLang.h)
B3DFindImportedLibraryWithConfigurationNames(glslang glslang STATIC glslang glslang${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang HLSL STATIC HLSL HLSL${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang OSDependent STATIC OSDependent OSDependent${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang OGLCompiler STATIC OGLCompiler OGLCompiler${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang SPIRV STATIC SPIRV SPIRV${glslang_DEBUG_SUFFIX})

B3DEndFindPackage(glslang glslang)
