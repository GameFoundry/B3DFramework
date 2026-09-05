# B3D Framework - Copyright 2026 Marko Pintera
# Licensed under the MIT license. See LICENSE.md for full terms.

# Find the DirectX Shader Compiler distributed with the Windows SDK.
#
# This module defines:
#  DirectXShaderCompiler_INCLUDE_DIRS
#  DirectXShaderCompiler_LIBRARIES
#  DirectXShaderCompiler_FOUND

B3DStartFindPackage(DirectXShaderCompiler)

cmake_host_system_information(
	RESULT DirectXShaderCompiler_WINDOWS_KITS_ROOT
	QUERY WINDOWS_REGISTRY "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
	VALUE KitsRoot10
	VIEW 32
)

if(DirectXShaderCompiler_WINDOWS_KITS_ROOT)
	cmake_path(CONVERT "${DirectXShaderCompiler_WINDOWS_KITS_ROOT}" TO_CMAKE_PATH_LIST DirectXShaderCompiler_WINDOWS_KITS_ROOT NORMALIZE)
endif()

if(B3D_IS_64BIT)
	set(DirectXShaderCompiler_ARCHITECTURE x64)
else()
	set(DirectXShaderCompiler_ARCHITECTURE x86)
endif()

# Prefer the selected SDK, then try installed SDKs from newest to oldest.
file(GLOB DirectXShaderCompiler_WINDOWS_SDK_VERSIONS RELATIVE "${DirectXShaderCompiler_WINDOWS_KITS_ROOT}/Include" "${DirectXShaderCompiler_WINDOWS_KITS_ROOT}/Include/10.*")
list(SORT DirectXShaderCompiler_WINDOWS_SDK_VERSIONS COMPARE NATURAL ORDER DESCENDING)
if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
	list(PREPEND DirectXShaderCompiler_WINDOWS_SDK_VERSIONS "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
endif()

foreach(DirectXShaderCompiler_WINDOWS_SDK_VERSION IN LISTS DirectXShaderCompiler_WINDOWS_SDK_VERSIONS)
	set(DirectXShaderCompiler_INCLUDE_DIRECTORY "${DirectXShaderCompiler_WINDOWS_KITS_ROOT}/Include/${DirectXShaderCompiler_WINDOWS_SDK_VERSION}/um")
	set(DirectXShaderCompiler_LIBRARY_DIRECTORY "${DirectXShaderCompiler_WINDOWS_KITS_ROOT}/Lib/${DirectXShaderCompiler_WINDOWS_SDK_VERSION}/um/${DirectXShaderCompiler_ARCHITECTURE}")
	set(DirectXShaderCompiler_BINARY_DIRECTORY "${DirectXShaderCompiler_WINDOWS_KITS_ROOT}/bin/${DirectXShaderCompiler_WINDOWS_SDK_VERSION}/${DirectXShaderCompiler_ARCHITECTURE}")

	if(EXISTS "${DirectXShaderCompiler_INCLUDE_DIRECTORY}/dxcapi.h" AND EXISTS "${DirectXShaderCompiler_LIBRARY_DIRECTORY}/dxcompiler.lib"
		AND EXISTS "${DirectXShaderCompiler_BINARY_DIRECTORY}/dxcompiler.dll" AND EXISTS "${DirectXShaderCompiler_BINARY_DIRECTORY}/dxil.dll")
		break()
	endif()
endforeach()

find_path(DirectXShaderCompiler_INCLUDE_DIR NAMES dxcapi.h PATHS "${DirectXShaderCompiler_INCLUDE_DIRECTORY}" NO_DEFAULT_PATH)
find_library(dxcompiler_LIBRARY NAMES dxcompiler PATHS "${DirectXShaderCompiler_LIBRARY_DIRECTORY}" NO_DEFAULT_PATH)
find_file(dxcompiler_BINARY NAMES dxcompiler.dll PATHS "${DirectXShaderCompiler_BINARY_DIRECTORY}" NO_DEFAULT_PATH)
find_file(dxil_BINARY NAMES dxil.dll PATHS "${DirectXShaderCompiler_BINARY_DIRECTORY}" NO_DEFAULT_PATH)

if(DirectXShaderCompiler_INCLUDE_DIR AND dxcompiler_LIBRARY AND dxcompiler_BINARY AND dxil_BINARY)
	set(DirectXShaderCompiler_FOUND TRUE)
	set(DirectXShaderCompiler_INCLUDE_DIRS "${DirectXShaderCompiler_INCLUDE_DIR}")

	B3DAddImportedLibrary(DirectXShaderCompiler::dxcompiler SHARED "${dxcompiler_LIBRARY}" "${dxcompiler_LIBRARY}" "${dxcompiler_BINARY}" "${dxcompiler_BINARY}")
	B3DAddImportedLibrary(DirectXShaderCompiler::dxil MODULE "" "" "${dxil_BINARY}" "${dxil_BINARY}")

	set_property(TARGET DirectXShaderCompiler::dxcompiler PROPERTY IMPORTED_GLOBAL TRUE)
	set_property(TARGET DirectXShaderCompiler::dxil PROPERTY IMPORTED_GLOBAL TRUE)
	set_property(TARGET DirectXShaderCompiler::dxcompiler PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DirectXShaderCompiler_INCLUDE_DIR}")
	set_property(TARGET DirectXShaderCompiler::dxcompiler PROPERTY B3D_DEPLOY_IMPORTED_RUNTIME TRUE)
	set_property(TARGET DirectXShaderCompiler::dxil PROPERTY B3D_DEPLOY_IMPORTED_RUNTIME TRUE)

	set(DirectXShaderCompiler_LIBRARIES DirectXShaderCompiler::dxcompiler DirectXShaderCompiler::dxil)
else()
	set(DirectXShaderCompiler_FOUND FALSE)
endif()

B3DEndFindPackage(DirectXShaderCompiler dxcompiler)

mark_as_advanced(
	DirectXShaderCompiler_WINDOWS_KITS_ROOT
	DirectXShaderCompiler_INCLUDE_DIR
	dxcompiler_LIBRARY
	dxcompiler_BINARY
	dxil_BINARY
)
