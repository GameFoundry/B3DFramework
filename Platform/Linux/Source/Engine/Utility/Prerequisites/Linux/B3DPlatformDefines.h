//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Linux compiler + platform-specific defines. Included by the global B3DPlatformDefines.h for the
// active platform; relies on the base ID constants and boolean convenience macros it defines first.

// Find the compiler type and pull in its defines.
#if defined(__clang__)
#	include "Prerequisites/Compiler/B3DCompilerClang.h"
#elif defined(__INTEL_COMPILER)
#	include "Prerequisites/Compiler/B3DCompilerIntel.h"
#elif defined(__GNUC__) // Check after Clang, as Clang defines this too
#	include "Prerequisites/Compiler/B3DCompilerGCC.h"
#else
#	error "No supported compiler for Linux. "
#endif

#ifdef DEBUG
#	define B3D_DEBUG 1
#else
#	define B3D_DEBUG 0
#endif

// Intel compiler defines thread-local storage differently based on platform.
#if B3D_COMPILER_INTEL
#	define B3D_THREADLOCAL __thread
#endif

#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32
#	define B3D_BREAK() __asm__ volatile("int $0x03")
#else
#	define B3D_BREAK() raise(SIGTRAP)
#endif

#define B3D_CODE_SECTION(name) __attribute__((section(name)))

// DLL export
#define B3D_EXPORT __attribute__((visibility("default")))
#define B3D_HIDDEN __attribute__((visibility("hidden")))

// Plugin export
#define B3D_PLUGIN_EXPORT __attribute__((visibility("default")))
#define B3D_PLUGIN_HIDDEN __attribute__((visibility("hidden")))

// Dynamic library naming
#define B3D_DYNLIB_EXTENSION "so"
#define B3D_DYNLIB_PREFIX "lib"
