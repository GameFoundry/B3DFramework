# Find glslang
#
# This module defines
#  glslang_INCLUDE_DIRS
#  glslang_LIBRARIES
#  glslang_FOUND

start_find_package(glslang)

if(B3D_USE_BUNDLED_LIBRARIES)
	set(glslang_INSTALL_DIR ${BSF_SOURCE_DIR}/../Dependencies/glslang CACHE PATH "")
endif()
gen_default_lib_search_dirs(glslang)

if(WIN32)
	set(glslang_DEBUG_SUFFIX "d")
else()
	set(glslang_DEBUG_SUFFIX "")
endif()

find_imported_includes(glslang glslang/Public/ShaderLang.h)
find_imported_library2(glslang glslang glslang${glslang_DEBUG_SUFFIX})
find_imported_library2(glslang HLSL HLSL${glslang_DEBUG_SUFFIX})
find_imported_library2(glslang OSDependent OSDependent${glslang_DEBUG_SUFFIX})
find_imported_library2(glslang OGLCompiler OGLCompiler${glslang_DEBUG_SUFFIX})
find_imported_library2(glslang SPIRV SPIRV${glslang_DEBUG_SUFFIX})

end_find_package(glslang glslang)
