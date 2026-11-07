# B3D Framework - Copyright 2026 Marko Pintera
# Licensed under the MIT license. See LICENSE.md for full terms.

# Find the DirectX Shader Compiler
#
# This module defines:
#  DirectXShaderCompiler_INCLUDE_DIRS
#  DirectXShaderCompiler_LIBRARIES
#  DirectXShaderCompiler_FOUND

# Each shader backend looks the package up from its own folder, and imported targets can only be created once
if(TARGET DirectXShaderCompiler::dxcompiler)
	set(DirectXShaderCompiler_FOUND TRUE)
	set(DirectXShaderCompiler_INCLUDE_DIRS ${DirectXShaderCompiler_INCLUDE_DIR})
	set(DirectXShaderCompiler_LIBRARIES DirectXShaderCompiler::dxcompiler)
	if(WIN32)
		list(APPEND DirectXShaderCompiler_LIBRARIES DirectXShaderCompiler::dxil)
	endif()

	return()
endif()

B3DStartFindPackage(DirectXShaderCompiler)

set(DirectXShaderCompiler_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/DirectXShaderCompiler)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT DirectXShaderCompiler_INSTALL_DIR)
	set(DirectXShaderCompiler_INSTALL_DIR ${DirectXShaderCompiler_BUNDLED_INSTALL_DIR} CACHE PATH "Path to DirectXShaderCompiler dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, fetching it with the build script if no prebuilt package is available
B3DEnsureBundledDependency(DirectXShaderCompiler BUILD_SCRIPT B3DBuildDirectXShaderCompiler.sh)

B3DPopulateDefaultPackageSearchPaths(DirectXShaderCompiler)

# Always resolved from the current install folder, as the Windows SDK provides another copy of this header
unset(DirectXShaderCompiler_INCLUDE_DIR CACHE)
find_path(DirectXShaderCompiler_INCLUDE_DIR NAMES dxcapi.h PATHS ${DirectXShaderCompiler_INCLUDE_SEARCH_DIRS} NO_DEFAULT_PATH)

if(DirectXShaderCompiler_INCLUDE_DIR)
	set(DirectXShaderCompiler_FOUND TRUE)
else()
	message(STATUS "...Cannot find include file \"dxcapi.h\" at path ${DirectXShaderCompiler_INCLUDE_SEARCH_DIRS}")
	set(DirectXShaderCompiler_FOUND FALSE)
endif()

B3DFindImportedLibrary(DirectXShaderCompiler dxcompiler SHARED)

# The DXIL validator is loaded by the compiler at runtime, and only DXIL code generation needs it
if(WIN32)
	B3DFindImportedLibrary(DirectXShaderCompiler dxil MODULE)
endif()

B3DEndFindPackage(DirectXShaderCompiler dxcompiler)

if(DirectXShaderCompiler_FOUND)
	foreach(library IN LISTS DirectXShaderCompiler_LIBRARIES)
		set_property(TARGET ${library} PROPERTY IMPORTED_GLOBAL TRUE)
		set_property(TARGET ${library} PROPERTY B3D_DEPLOY_IMPORTED_RUNTIME TRUE)
	endforeach()
endif()
