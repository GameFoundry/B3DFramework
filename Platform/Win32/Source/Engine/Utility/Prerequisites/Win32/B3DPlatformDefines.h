//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Win32 compiler + platform-specific defines. Included by the global B3DPlatformDefines.h for the
// active platform; relies on the base ID constants and boolean convenience macros it defines first.

// Find the compiler type and pull in its defines.
#if defined(__clang__)
#	include "Prerequisites/Compiler/B3DCompilerClang.h"
#elif defined(__INTEL_COMPILER)
#	include "Prerequisites/Compiler/B3DCompilerIntel.h"
#elif defined(_MSC_VER) // Check after Clang and Intel, since we could be building with either within VS
#	include "Prerequisites/Compiler/B3DCompilerMSVC.h"
#else
#	error "No supported compiler for Win32. "
#endif

#include <intrin.h>

// Win32 compilers use _DEBUG for specifying debug builds. For MinGW, we set DEBUG.
#if defined(_DEBUG) || defined(DEBUG)
#	define B3D_DEBUG 1
#else
#	define B3D_DEBUG 0
#endif

// Intel compiler defines thread-local storage differently based on platform.
#if B3D_COMPILER_INTEL
#	define B3D_THREADLOCAL __declspec(thread)
#endif

#define B3D_BREAK() (__nop(), __debugbreak())
#define B3D_CODE_SECTION(name) __declspec(code_seg(name))

// DLL export
#if B3D_COMPILER_MSVC
#	if defined(B3D_STATIC_LIB)
#		define B3D_EXPORT
#	else
#		if defined(B3D_EXPORTS)
#			define B3D_EXPORT __declspec(dllexport)
#		else
#			define B3D_EXPORT __declspec(dllimport)
#		endif
#	endif
#else
#	if defined(B3D_STATIC_LIB)
#		define B3D_EXPORT
#	else
#		if defined(B3D_EXPORTS)
#			define B3D_EXPORT __attribute__((dllexport))
#		else
#			define B3D_EXPORT __attribute__((dllimport))
#		endif
#	endif
#endif
#define B3D_HIDDEN

// Plugin export
#if B3D_COMPILER_MSVC
#	define B3D_PLUGIN_EXPORT __declspec(dllexport)
#else
#	define B3D_PLUGIN_EXPORT __attribute__((dllexport))
#endif
#define B3D_PLUGIN_HIDDEN

// Dynamic library naming
#define B3D_DYNLIB_EXTENSION "dll"
#define B3D_DYNLIB_PREFIX nullptr
