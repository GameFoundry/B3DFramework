# Find glslang
#
# This module defines
#  glslang_INCLUDE_DIRS
#  glslang_LIBRARIES
#  glslang_FOUND

B3DStartFindPackage(glslang)

set(glslang_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/glslang)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT glslang_INSTALL_DIR)
	set(glslang_INSTALL_DIR ${glslang_BUNDLED_INSTALL_DIR} CACHE PATH "Path to glslang dependency" FORCE)
endif()
B3DPopulateDefaultPackageSearchPaths(glslang)

if(WIN32)
	set(glslang_DEBUG_SUFFIX "d")
else()
	set(glslang_DEBUG_SUFFIX "")
endif()

B3DFindImportedIncludes(glslang glslang/Public/ShaderLang.h)
B3DFindImportedLibraryWithConfigurationNames(glslang SPIRV STATIC SPIRV SPIRV${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang glslang STATIC glslang glslang${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang glslang-default-resource-limits STATIC glslang-default-resource-limits glslang-default-resource-limits${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang MachineIndependent STATIC MachineIndependent MachineIndependent${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang GenericCodeGen STATIC GenericCodeGen GenericCodeGen${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang OSDependent STATIC OSDependent OSDependent${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang SPIRV-Tools-opt STATIC SPIRV-Tools-opt SPIRV-Tools-opt${glslang_DEBUG_SUFFIX})
B3DFindImportedLibraryWithConfigurationNames(glslang SPIRV-Tools STATIC SPIRV-Tools SPIRV-Tools${glslang_DEBUG_SUFFIX})

B3DEndFindPackage(glslang glslang)
