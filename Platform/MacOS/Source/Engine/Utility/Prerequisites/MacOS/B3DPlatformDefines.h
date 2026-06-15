//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// macOS compiler + platform-specific defines. Included by the global B3DPlatformDefines.h for the
// active platform; relies on the base ID constants and boolean convenience macros it defines first.

// Find the compiler type and pull in its defines.
#if defined(__clang__)
#	include "Prerequisites/Compiler/B3DCompilerClang.h"
#else
#	error "No supported compiler for macOS. "
#endif

#ifdef DEBUG
#	define B3D_DEBUG 1
#else
#	define B3D_DEBUG 0
#endif

#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32
#	define B3D_BREAK() __asm__("int $3")
#else
#	define B3D_BREAK() __builtin_trap()
#endif

#define B3D_CODE_SECTION(name) __attribute__((section("__TEXT,__" name ",regular,pure_instructions")))

// DLL export
#define B3D_EXPORT __attribute__((visibility("default")))
#define B3D_HIDDEN __attribute__((visibility("hidden")))

// Plugin export
#define B3D_PLUGIN_EXPORT __attribute__((visibility("default")))
#define B3D_PLUGIN_HIDDEN __attribute__((visibility("hidden")))

// Dynamic library naming
#define B3D_DYNLIB_EXTENSION "dylib"
#define B3D_DYNLIB_PREFIX "lib"
