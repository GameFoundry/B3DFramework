# Find glslang
#
# This module defines
#  glslang_INCLUDE_DIRS
#  glslang_LIBRARIES
#  glslang_FOUND

set(glslang_INSTALL_DIRS ${PROJECT_SOURCE_DIR}/../Dependencies/glslang CACHE PATH "")
set(glslang_INCLUDE_SEARCH_DIRS "${glslang_INSTALL_DIRS}/include")

if(BS_64BIT)
	set(glslang_LIBRARY_SEARCH_DIRS "${glslang_INSTALL_DIRS}/lib/x64")
else()
	set(glslang_LIBRARY_SEARCH_DIRS "${glslang_INSTALL_DIRS}/lib/x86")
endif()

message(STATUS "Looking for glslang installation...")
find_path(glslang_INCLUDE_DIR glslang/Public/ShaderLang.h PATHS ${glslang_INCLUDE_SEARCH_DIRS})

find_library(glslang_LIBRARY_OPTIMIZED NAMES glslang PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangHLSL_LIBRARY_OPTIMIZED NAMES HLSL PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangOSDependent_LIBRARY_OPTIMIZED NAMES OSDependent PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangOGLCompiler_LIBRARY_OPTIMIZED NAMES OGLCompiler PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangSPIRV_LIBRARY_OPTIMIZED NAMES SPIRV PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslang_LIBRARY_DEBUG NAMES glslangd glslang PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangHLSL_LIBRARY_DEBUG NAMES HLSLd HLSL PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangOSDependent_LIBRARY_DEBUG NAMES OSDependentd OSDependent PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangOGLCompiler_LIBRARY_DEBUG NAMES OGLCompilerd OGLCompiler PATHS ${glslang_LIBRARY_SEARCH_DIRS})
find_library(glslangSPIRV_LIBRARY_DEBUG NAMES SPIRVd PATHS ${glslang_LIBRARY_SEARCH_DIRS})

if(glslang_INCLUDE_DIR AND 
	glslang_LIBRARY_OPTIMIZED AND glslang_LIBRARY_DEBUG AND
	glslangHLSL_LIBRARY_OPTIMIZED AND glslangHLSL_LIBRARY_DEBUG AND
	glslangOSDependent_LIBRARY_OPTIMIZED AND glslangOSDependent_LIBRARY_DEBUG AND
	glslangOGLCompiler_LIBRARY_OPTIMIZED AND glslangOGLCompiler_LIBRARY_DEBUG AND
	glslangSPIRV_LIBRARY_OPTIMIZED AND glslangSPIRV_LIBRARY_DEBUG
	)
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

if(glslang_FOUND)
	add_library(glslang STATIC IMPORTED)
	set_target_properties(glslang PROPERTIES IMPORTED_LOCATION_DEBUG "${glslang_LIBRARY_DEBUG}")
	set_target_properties(glslang PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${glslang_LIBRARY_DEBUG}")
	set_target_properties(glslang PROPERTIES IMPORTED_LOCATION_RELEASE "${glslang_LIBRARY_OPTIMIZED}")
	
	add_library(glslangHLSL STATIC IMPORTED)
	set_target_properties(glslangHLSL PROPERTIES IMPORTED_LOCATION_DEBUG "${glslangHLSL_LIBRARY_DEBUG}")
	set_target_properties(glslangHLSL PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${glslangHLSL_LIBRARY_DEBUG}")
	set_target_properties(glslangHLSL PROPERTIES IMPORTED_LOCATION_RELEASE "${glslangHLSL_LIBRARY_OPTIMIZED}")

	add_library(glslangOSDependent STATIC IMPORTED)
	set_target_properties(glslangOSDependent PROPERTIES IMPORTED_LOCATION_DEBUG "${glslangOSDependent_LIBRARY_DEBUG}")
	set_target_properties(glslangOSDependent PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${glslangOSDependent_LIBRARY_DEBUG}")
	set_target_properties(glslangOSDependent PROPERTIES IMPORTED_LOCATION_RELEASE "${glslangOSDependent_LIBRARY_OPTIMIZED}")

	add_library(glslangOGLCompiler STATIC IMPORTED)
	set_target_properties(glslangOGLCompiler PROPERTIES IMPORTED_LOCATION_DEBUG "${glslangOGLCompiler_LIBRARY_DEBUG}")
	set_target_properties(glslangOGLCompiler PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${glslangOGLCompiler_LIBRARY_DEBUG}")
	set_target_properties(glslangOGLCompiler PROPERTIES IMPORTED_LOCATION_RELEASE "${glslangOGLCompiler_LIBRARY_OPTIMIZED}")

	add_library(glslangSPIRV STATIC IMPORTED)
	set_target_properties(glslangSPIRV PROPERTIES IMPORTED_LOCATION_DEBUG "${glslangSPIRV_LIBRARY_DEBUG}")
	set_target_properties(glslangSPIRV PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${glslangSPIRV_LIBRARY_DEBUG}")
	set_target_properties(glslangSPIRV PROPERTIES IMPORTED_LOCATION_RELEASE "${glslangSPIRV_LIBRARY_OPTIMIZED}")	
endif()

mark_as_advanced(
	glslang_INSTALL_DIRS 
	glslang_INCLUDE_DIR 
	glslang_LIBRARY_OPTIMIZED 
	glslangHLSL_LIBRARY_OPTIMIZED 
	glslangOSDependent_LIBRARY_OPTIMIZED
	glslangSPIRV_LIBRARY_OPTIMIZED
	glslangOGLCompiler_LIBRARY_OPTIMIZED
	glslang_LIBRARY_DEBUG
	glslangHLSL_LIBRARY_DEBUG
	glslangOSDependent_LIBRARY_DEBUG
	glslangOGLCompiler_LIBRARY_DEBUG
	glslangSPIRV_LIBRARY_DEBUG)

set(glslang_INCLUDE_DIRS ${glslang_INCLUDE_DIR})
set(glslang_LIBRARIES glslang glslangHLSL glslangOSDependent glslangOGLCompiler glslangSPIRV)