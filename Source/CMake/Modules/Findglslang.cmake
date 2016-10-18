# Find glslang
#
# This module defines
#  glslang_INCLUDE_DIRS
#  glslang_LIBRARIES_DEBUG
#  glslang_LIBRARIES_OPTIMIZED
#  glslang_LIBRARY_DIRS
#  glslang_FOUND

set(glslang_INSTALL_DIRS ${PROJECT_SOURCE_DIR}/../Dependencies/glslang CACHE PATH "")
set(glslang_INCLUDE_SEARCH_DIRS "${glslang_INSTALL_DIRS}/include")

if(BS_64BIT)
	set(glslang_LIBRARY_SEARCH_DIRS "${glslang_INSTALL_DIRS}/lib/x64")
else()
	set(glslang_LIBRARY_SEARCH_DIRS "${glslang_INSTALL_DIRS}/lib/x86")
endif()

message(STATUS "Looking for glslang installation...")
find_path(glslang_INCLUDE_DIR Public/ShaderLang.h PATHS ${glslang_INCLUDE_SEARCH_DIRS})

find_library(glslang_LIBRARY_OPTIMIZED NAMES glslang HLSL OSDependent OGLCompiler PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslang_LIBRARY_DEBUG NAMES glslangd HLSLd OSDependentd OGLCompilerd PATHS ${glslang_LIBRARY_SEARCH_DIRS})

if(glslang_INCLUDE_DIR AND glslang_LIBRARY_OPTIMIZED AND glslang_LIBRARY_DEBUG)
	set(glslang_FOUND TRUE)
endif()

if(NOT glslang_FOUND)
	if(glslang_FIND_REQUIRED)
		message(FATAL_ERROR "Cannot find glslang installation. Try modifying the glslang_INSTALL_DIRS path.")
	else()
		message(WARNING "Cannot find glslang installation. Try modifying the glslang_INSTALL_DIRS path.")
	endif()
else()
	message(STATUS "...glslang OK.")
endif()

mark_as_advanced(glslang_INSTALL_DIRS glslang_INCLUDE_DIR glslang_LIBRARY_OPTIMIZED glslang_LIBRARY_DEBUG)
set(glslang_INCLUDE_DIRS ${glslang_INCLUDE_DIR})
set(glslang_LIBRARIES_OPTIMIZED ${glslang_LIBRARY_OPTIMIZED})
set(glslang_LIBRARIES_DEBUG ${glslang_LIBRARY_DEBUG})