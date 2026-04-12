//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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

#define B3D_ARCHITECTURE_ID_X86_32 1
#define B3D_ARCHITECTURE_ID_X86_64 2
#define B3D_ARCHITECTURE_ID_ARM64 3

#define B3D_ENDIAN_ID_LITTLE 1
#define B3D_ENDIAN_BIG 2
#define B3D_ENDIAN B3D_ENDIAN_ID_LITTLE

#define B3D_INLINE inline

// Finds the compiler type and version.
#if defined(__clang__)
#	define B3D_COMPILER B3D_COMPILER_ID_CLANG
#	define B3D_COMPILER_VERSION __clang_version__
#	define B3D_THREADLOCAL __thread
#	define B3D_STDCALL __attribute__((stdcall))
#	define B3D_CDECL __attribute__((cdecl))
#	define B3D_FALLTHROUGH [[clang::fallthrough]];
#	define B3D_FORCENOINLINE __attribute__((noinline))
#	define B3D_FORCEINLINE inline __attribute__((always_inline))
#	define B3D_LIKELY(x) __builtin_expect(!!(x), 1)
#	define B3D_UNLIKELY(x) __builtin_expect(!!(x), 0)
#	define _B3D_DISABLE_OPTIMIZATION __pragma(clang optimize off)
#	define _B3D_ENABLE_OPTIMIZATION __pragma(clang optimize on)
#elif defined(__GNUC__) // Check after Clang, as Clang defines this too
#	define B3D_COMPILER B3D_COMPILER_ID_GCC
#	define B3D_COMPILER_VERSION (((__GNUC__)*100) + (__GNUC_MINOR__ * 10) + __GNUC_PATCHLEVEL__)
#	define B3D_THREADLOCAL __thread
#	define B3D_STDCALL __attribute__((stdcall))
#	define B3D_CDECL __attribute__((cdecl))
#	define B3D_FALLTHROUGH __attribute__((fallthrough));
#	define B3D_FORCENOINLINE __attribute__((noinline))
#	define B3D_FORCEINLINE inline __attribute__((always_inline))
#	define B3D_LIKELY(x) __builtin_expect(!!(x), 1)
#	define B3D_UNLIKELY(x) __builtin_expect(!!(x), 0)
#	define _B3D_DISABLE_OPTIMIZATION _Pragma("GCC optimize \"O0\"")
#	define _B3D_ENABLE_OPTIMIZATION _Pragma("GCC reset_options")
#elif defined(__INTEL_COMPILER)
#	define B3D_COMPILER B3D_COMPILER_ID_INTEL
#	define B3D_COMPILER_VERSION __INTEL_COMPILER
#	define B3D_STDCALL __stdcall
#	define B3D_CDECL __cdecl
#	define B3D_FALLTHROUGH
#	define _B3D_DISABLE_OPTIMIZATION __pragma(optimize("", off))
#	define _B3D_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#	define B3D_LIKELY(x) (x)
#	define B3D_UNLIKELY(x) (x)
// B3D_THREADLOCAL define is down below because Intel compiler defines it differently based on platform
#elif defined(_MSC_VER) // Check after Clang and Intel, since we could be building with either within VS
#	define B3D_COMPILER B3D_COMPILER_ID_MSVC
#	define B3D_COMPILER_VERSION _MSC_VER
#	define B3D_THREADLOCAL __declspec(thread)
#	define B3D_STDCALL __stdcall
#	define B3D_CDECL __cdecl
#	define B3D_FALLTHROUGH
#	define B3D_FORCENOINLINE __declspec(noinline)
#	define B3D_FORCEINLINE __forceinline
#	define B3D_LIKELY(x) (x)
#	define B3D_UNLIKELY(x) (x)
#	define _B3D_DISABLE_OPTIMIZATION __pragma(optimize("", off))
#	define _B3D_ENABLE_OPTIMIZATION __pragma(optimize("", on))
#	undef __PRETTY_FUNCTION__
#	define __PRETTY_FUNCTION__ __FUNCSIG__
#else
#	pragma error "No known compiler. "
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)
#	define B3D_PRETTY_FUNCTION __PRETTY_FUNCTION__
#	define B3D_PRETTY_FUNCTION_PREFIX '='
#	define B3D_PRETTY_FUNCTION_SUFFIX ']'
#elif defined(_MSC_VER)
#	define B3D_PRETTY_FUNCTION __FUNCSIG__
#	define B3D_PRETTY_FUNCTION_PREFIX '<'
#	define B3D_PRETTY_FUNCTION_SUFFIX '<'
#endif

#if !B3D_BUILD_TYPE_SHIPPING
#	define B3D_DISABLE_OPTIMIZATION _B3D_DISABLE_OPTIMIZATION
#	define B3D_ENABLE_OPTIMIZATION _B3D_ENABLE_OPTIMIZATION
#else
#	define B3D_DISABLE_OPTIMIZATION
#	define B3D_ENABLE_OPTIMIZATION
#endif

// Finds the current platform
#if defined(__WIN32__) || defined(_WIN32)
#	define B3D_PLATFORM B3D_PLATFORM_ID_WIN32
#elif defined(__APPLE_CC__)
#	define B3D_PLATFORM B3D_PLATFORM_ID_MACOS
#else
#	define B3D_PLATFORM B3D_PLATFORM_ID_LINUX
#endif

// Boolean convenience macros
#define B3D_PLATFORM_WIN32 (B3D_PLATFORM == B3D_PLATFORM_ID_WIN32)
#define B3D_PLATFORM_LINUX (B3D_PLATFORM == B3D_PLATFORM_ID_LINUX)
#define B3D_PLATFORM_MACOS (B3D_PLATFORM == B3D_PLATFORM_ID_MACOS)
#define B3D_PLATFORM_IOS (B3D_PLATFORM == B3D_PLATFORM_ID_IOS)
#define B3D_PLATFORM_ANDROID (B3D_PLATFORM == B3D_PLATFORM_ID_ANDROID)

// Boolean convenience macros for compilers
#define B3D_COMPILER_MSVC (B3D_COMPILER == B3D_COMPILER_ID_MSVC)
#define B3D_COMPILER_GCC (B3D_COMPILER == B3D_COMPILER_ID_GCC)
#define B3D_COMPILER_INTEL (B3D_COMPILER == B3D_COMPILER_ID_INTEL)
#define B3D_COMPILER_CLANG (B3D_COMPILER == B3D_COMPILER_ID_CLANG)

// Find the architecture type
#if defined(__x86_64__) || defined(_M_X64)
#	define B3D_ARCHITECTURE B3D_ARCHITECTURE_ID_X86_64
#elif defined(__aarch64__)
#	define R3D_ARCHITECTURE B3D_ARCHITECTURE_ID_ARM64
#else
#	define B3D_ARCHITECTURE B3D_ARCHITECTURE_ID_X86_32
#endif

// DLL export
#if B3D_PLATFORM_WIN32 // Windows
#	if B3D_COMPILER_MSVC
#		if defined(B3D_STATIC_LIB)
#			define B3D_EXPORT
#		else
#			if defined(B3D_EXPORTS)
#				define B3D_EXPORT __declspec(dllexport)
#			else
#				define B3D_EXPORT __declspec(dllimport)
#			endif
#		endif
#	else
#		if defined(B3D_STATIC_LIB)
#			define B3D_EXPORT
#		else
#			if defined(B3D_EXPORTS)
#				define B3D_EXPORT __attribute__((dllexport))
#			else
#				define B3D_EXPORT __attribute__((dllimport))
#			endif
#		endif
#	endif
#	define B3D_HIDDEN
#else // Linux/Mac settings
#	define B3D_EXPORT __attribute__((visibility("default")))
#	define B3D_HIDDEN __attribute__((visibility("hidden")))
#endif

// DLL export for plugins
#if B3D_MONOLITHIC_BUILD
#	define B3D_PLUGIN_EXPORT
#	define B3D_PLUGIN_HIDDEN
#elif B3D_PLATFORM_WIN32 // Windows
#	if B3D_COMPILER_MSVC
#		define B3D_PLUGIN_EXPORT __declspec(dllexport)
#	else
#		define B3D_PLUGIN_EXPORT __attribute__((dllexport))
#	endif
#	define B3D_PLUGIN_HIDDEN
#else // Linux/Mac settings
#	define B3D_PLUGIN_EXPORT __attribute__((visibility("default")))
#	define B3D_PLUGIN_HIDDEN __attribute__((visibility("hidden")))
#endif

// Windows Settings
#if B3D_PLATFORM_WIN32

#include <intrin.h>

// Win32 compilers use _DEBUG for specifying debug builds.
// for MinGW, we set DEBUG
#	if defined(_DEBUG) || defined(DEBUG)
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#	if B3D_COMPILER_INTEL
#		define B3D_THREADLOCAL __declspec(thread)
#	endif

#	define B3D_BREAK() (__nop(), __debugbreak())
#	define B3D_CODE_SECTION(name) __declspec(code_seg(name)) 
#endif

// Linux
#if B3D_PLATFORM_LINUX
#	ifdef DEBUG
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#	if B3D_COMPILER_INTEL
#		define B3D_THREADLOCAL __thread
#	endif

#	if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32
#		define B3D_BREAK() __asm__ volatile("int $0x03")
#	else
#		define B3D_BREAK() raise(SIGTRAP)
#	endif

#	define B3D_CODE_SECTION(name) __attribute__((section(name)))
#endif

// Mac Settings
#if B3D_PLATFORM_MACOS
#	ifdef DEBUG
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#	if B3D_COMPILER_INTEL
#		define B3D_THREADLOCAL __thread
#	endif

#	if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32
#		define B3D_BREAK() __asm__("int $3")
#	else
#		define B3D_BREAK() __builtin_trap()
#	endif

#	define B3D_CODE_SECTION(name) __attribute__((section("__TEXT,__" name ",regular,pure_instructions")))
#endif

// iOS
#if B3D_PLATFORM_IOS
#	define B3D_BREAK() __builtin_trap()
#	define B3D_CODE_SECTION(name) __attribute__((section("__TEXT,__" name ",regular,pure_instructions"))) __attribute__((aligned(4)))
#endif

// Android
#if B3D_PLATFORM_ANDROID
#	define B3D_BREAK() raise(SIGTRAP)
#	define B3D_CODE_SECTION(name) __attribute__((section(name)))
#endif
