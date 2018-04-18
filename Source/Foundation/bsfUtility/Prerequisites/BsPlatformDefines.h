//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Initial platform/compiler-related stuff to set.
#define BS_PLATFORM_WIN32 1
#define BS_PLATFORM_LINUX 2
#define BS_PLATFORM_OSX 3
#define BS_PLATFORM_IOS 4
#define BS_PLATFORM_ANDROID 5

#define BS_COMPILER_MSVC 1
#define BS_COMPILER_GNUC 2
#define BS_COMPILER_INTEL 3
#define BS_COMPILER_CLANG 4

#define BS_ARCHITECTURE_x86_32 1
#define BS_ARCHITECTURE_x86_64 2

#define BS_ENDIAN_LITTLE 1
#define BS_ENDIAN_BIG 2
#define BS_ENDIAN BS_ENDIAN_LITTLE

// Finds the compiler type and version.
#if defined(__clang__)
#	define BS_COMPILER BS_COMPILER_CLANG
#	define BS_COMP_VER __clang_version__
#	define BS_THREADLOCAL __thread
#	define BS_STDCALL __attribute__((stdcall))
#	define BS_CDECL __attribute__((cdecl))
#	define BS_FALLTHROUGH [[clang::fallthrough]];
#elif defined(__GNUC__) // Check after Clang, as Clang defines this too
#	define BS_COMPILER BS_COMPILER_GNUC
#	define BS_COMP_VER (((__GNUC__)*100) + \
		(__GNUC_MINOR__*10) + \
		__GNUC_PATCHLEVEL__)
#	define BS_THREADLOCAL __thread
#	define BS_STDCALL __attribute__((stdcall))
#	define BS_CDECL __attribute__((cdecl))
#	define BS_FALLTHROUGH __attribute__((fallthrough));
#elif defined (__INTEL_COMPILER)
#	define BS_COMPILER BS_COMPILER_INTEL
#	define BS_COMP_VER __INTEL_COMPILER
#	define BS_STDCALL __stdcall
#	define BS_CDECL __cdecl
#	define BS_FALLTHROUGH
	// BS_THREADLOCAL define is down below because Intel compiler defines it differently based on platform
#elif defined(_MSC_VER) // Check after Clang and Intel, since we could be building with either within VS
#	define BS_COMPILER BS_COMPILER_MSVC
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
#if defined( __WIN32__ ) || defined( _WIN32 )
#	define BS_PLATFORM BS_PLATFORM_WIN32
#elif defined( __APPLE_CC__)
#	define BS_PLATFORM BS_PLATFORM_OSX
#else
#	define BS_PLATFORM BS_PLATFORM_LINUX
#endif

// Find the architecture type
#if defined(__x86_64__) || defined(_M_X64)
#	define BS_ARCH_TYPE BS_ARCHITECTURE_x86_64
#else
#	define BS_ARCH_TYPE BS_ARCHITECTURE_x86_32
#endif

// DLL export
#if BS_PLATFORM == BS_PLATFORM_WIN32 // Windows
#	if BS_COMPILER == BS_COMPILER_MSVC
#		if defined(BS_STATIC_LIB)
#			define BS_UTILITY_EXPORT
#		else
#			if defined(BS_UTILITY_EXPORTS)
#				define BS_UTILITY_EXPORT __declspec(dllexport)
#			else
#				define BS_UTILITY_EXPORT __declspec(dllimport)
#			endif
#		endif
#	else
#		if defined(BS_STATIC_LIB)
#			define BS_UTILITY_EXPORT
#		else
#			if defined(BS_UTILITY_EXPORTS)
#				define BS_UTILITY_EXPORT __attribute__ ((dllexport))
#			else
#				define BS_UTILITY_EXPORT __attribute__ ((dllimport))
#			endif
#		endif
#	endif
#define BS_UTILITY_HIDDEN
#else // Linux/Mac settings
#	define BS_UTILITY_EXPORT __attribute__ ((visibility ("default")))
#	define BS_UTILITY_HIDDEN __attribute__ ((visibility ("hidden")))
#endif

// DLL export for plugins
#if BS_PLATFORM == BS_PLATFORM_WIN32 // Windows
#	if BS_COMPILER == BS_COMPILER_MSVC
#		define BS_PLUGIN_EXPORT __declspec(dllexport)
#	else
#		define BS_PLUGIN_EXPORT __attribute__ ((dllexport))
#	endif
#	define BS_UTILITY_HIDDEN
#else // Linux/Mac settings
#	define BS_PLUGIN_EXPORT __attribute__ ((visibility ("default")))
#endif

// Windows Settings
#if BS_PLATFORM == BS_PLATFORM_WIN32
// Win32 compilers use _DEBUG for specifying debug builds.
// for MinGW, we set DEBUG
#	if defined(_DEBUG) || defined(DEBUG)
#		define BS_DEBUG_MODE 1
#	else
#		define BS_DEBUG_MODE 0
#	endif

#	if BS_COMPILER == BS_COMPILER_INTEL
#		define BS_THREADLOCAL __declspec(thread)
#	endif
#endif

// Linux/Mac Settings
#if BS_PLATFORM == BS_PLATFORM_LINUX || BS_PLATFORM == BS_PLATFORM_OSX
#	ifdef DEBUG
#		define BS_DEBUG_MODE 1
#	else
#	define BS_DEBUG_MODE 0
#	endif

#	if BS_COMPILER == BS_COMPILER_INTEL
#		define BS_THREADLOCAL __thread
#	endif
#endif