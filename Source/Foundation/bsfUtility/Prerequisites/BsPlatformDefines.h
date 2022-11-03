//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Initial platform/compiler-related stuff to set.
#define B3D_PLATFORM_ID_WIN32 1
#define B3D_PLATFORM_ID_LINUX 2
#define B3D_PLATFORM_ID_MACOS 3
#define B3D_PLATFORM_ID_IOS 4
#define B3D_PLATFORM_ID_ANDROID 5

#define B3D_COMPILER_ID_MSVC 1
#define B3D_COMPILER_ID_GCC 2
#define B3D_COMPILER_ID_INTEL 3
#define B3D_COMPILER_ID_CLANG 4

#define B3D_ARCHITECTURE_ID_x86_32 1
#define B3D_ARCHITECTURE_ID_x86_64 2

#define BS_ENDIAN_LITTLE 1
#define BS_ENDIAN_BIG 2
#define BS_ENDIAN BS_ENDIAN_LITTLE

// Finds the compiler type and version.
#if defined(__clang__)
#	define B3D_COMPILER B3D_COMPILER_ID_CLANG
#	define BS_COMP_VER __clang_version__
#	define BS_THREADLOCAL __thread
#	define BS_STDCALL __attribute__((stdcall))
#	define BS_CDECL __attribute__((cdecl))
#	define BS_FALLTHROUGH [[clang::fallthrough]];
#elif defined(__GNUC__) // Check after Clang, as Clang defines this too
#	define B3D_COMPILER B3D_COMPILER_ID_GCC
#	define BS_COMP_VER (((__GNUC__)*100) + (__GNUC_MINOR__ * 10) + __GNUC_PATCHLEVEL__)
#	define BS_THREADLOCAL __thread
#	define BS_STDCALL __attribute__((stdcall))
#	define BS_CDECL __attribute__((cdecl))
#	define BS_FALLTHROUGH __attribute__((fallthrough));
#elif defined(__INTEL_COMPILER)
#	define B3D_COMPILER B3D_COMPILER_ID_INTEL
#	define BS_COMP_VER __INTEL_COMPILER
#	define BS_STDCALL __stdcall
#	define BS_CDECL __cdecl
#	define BS_FALLTHROUGH
// BS_THREADLOCAL define is down below because Intel compiler defines it differently based on platform
#elif defined(_MSC_VER) // Check after Clang and Intel, since we could be building with either within VS
#	define B3D_COMPILER B3D_COMPILER_ID_MSVC
#	define BS_COMP_VER _MSC_VER
#	define BS_THREADLOCAL __declspec(thread)
#	define BS_STDCALL __stdcall
#	define BS_CDECL __cdecl
#	define BS_FALLTHROUGH
#	undef __PRETTY_FUNCTION__
#	define __PRETTY_FUNCTION__ __FUNCSIG__
#else
#	pragma error "No known compiler. "
#endif

// Finds the current platform
#if defined(__WIN32__) || defined(_WIN32)
#	define B3D_PLATFORM B3D_PLATFORM_ID_WIN32
#elif defined(__APPLE_CC__)
#	define B3D_PLATFORM B3D_PLATFORM_ID_MACOS
#else
#	define B3D_PLATFORM B3D_PLATFORM_ID_LINUX
#endif

// Find the architecture type
#if defined(__x86_64__) || defined(_M_X64)
#	define B3D_ARCHITECTURE B3D_ARCHITECTURE_ID_x86_64
#else
#	define B3D_ARCHITECTURE B3D_ARCHITECTURE_ID_x86_32
#endif

// DLL export
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32 // Windows
#	if B3D_COMPILER == B3D_COMPILER_ID_MSVC
#		if defined(BS_STATIC_LIB)
#			define B3D_UTILITY_EXPORT
#		else
#			if defined(B3D_UTILITY_EXPORTS)
#				define B3D_UTILITY_EXPORT __declspec(dllexport)
#			else
#				define B3D_UTILITY_EXPORT __declspec(dllimport)
#			endif
#		endif
#	else
#		if defined(BS_STATIC_LIB)
#			define B3D_UTILITY_EXPORT
#		else
#			if defined(B3D_UTILITY_EXPORTS)
#				define B3D_UTILITY_EXPORT __attribute__((dllexport))
#			else
#				define B3D_UTILITY_EXPORT __attribute__((dllimport))
#			endif
#		endif
#	endif
#	define BS_UTILITY_HIDDEN
#else // Linux/Mac settings
#	define B3D_UTILITY_EXPORT __attribute__((visibility("default")))
#	define BS_UTILITY_HIDDEN __attribute__((visibility("hidden")))
#endif

// DLL export for plugins
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32 // Windows
#	if B3D_COMPILER == B3D_COMPILER_ID_MSVC
#		define B3D_PLUGIN_EXPORT __declspec(dllexport)
#	else
#		define B3D_PLUGIN_EXPORT __attribute__((dllexport))
#	endif
#	define BS_UTILITY_HIDDEN
#else // Linux/Mac settings
#	define B3D_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Windows Settings
#if B3D_PLATFORM == B3D_PLATFORM_ID_WIN32
// Win32 compilers use _DEBUG for specifying debug builds.
// for MinGW, we set DEBUG
#	if defined(_DEBUG) || defined(DEBUG)
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#	if B3D_COMPILER == B3D_COMPILER_ID_INTEL
#		define BS_THREADLOCAL __declspec(thread)
#	endif
#endif

// Linux/Mac Settings
#if B3D_PLATFORM == B3D_PLATFORM_ID_LINUX || B3D_PLATFORM == B3D_PLATFORM_ID_MACOS
#	ifdef DEBUG
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#	if B3D_COMPILER == B3D_COMPILER_ID_INTEL
#		define BS_THREADLOCAL __thread
#	endif
#endif
