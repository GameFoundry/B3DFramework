//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <assert.h>

/** @defgroup Utility Utility
 *	Lowest layer of the engine containing various utility and helper classes.
 *  @{
 */

/** @defgroup Containers Containers
 *  Templated commonly used containers.
 */

/** @defgroup Debug Debug
 *  Various debugging helpers.
 */

/** @defgroup Error Error handling
 *  Handling and reporting errors.
 */

/** @defgroup Filesystem File system
 *  Manipulating, reading and writing files.
 */

/** @defgroup General General
 *  General utility functionality that doesn't fit in any other category.
 */

/** @defgroup Image Image
 *  Manipulating images.
 */

/** @defgroup Math Math
 *	Variety of general purpose math functionality.
 */

/** @defgroup Memory Memory
 *  Allocators, deallocators and memory manipulation.
 */

/** @defgroup Platform-Utility Platform
 *  %Platform specific functionality.
 */

/** @defgroup RTTI RTTI
 *  Run-time type information defining and querying.
 */

/** @cond RTTI */
/** @defgroup RTTI-Impl-Utility RTTI types
 *  RTTI implementations for classes within the utility layer.
 */
/** @endcond */

/** @defgroup Serialization Serialization
 *  Serialization and deserialization of native objects.
 */

/** @defgroup String String
 *  String manipulation.
 */

/** @defgroup Testing Testing
 *  Running unit tests.
 */

/** @defgroup Threading Threading
 *  Thread manipulation and synchronization.
 */

/** @defgroup ECS ECS
  *	All low-level functionality for creating and using the entity component system.
  */

/** @} */

/** @defgroup Internals Internals
 *	Non-user-facing low-level classes and methods, useful primarily to those modifying the engine.
 *  @{
 */

/** @defgroup Utility-Internal Utility
 *	Lowest layer of the engine containing various utility and helper classes.
 *  @{
 */

/** @defgroup Error-Internal Error handling
 *  Handling and reporting errors.
 */

/** @defgroup General-Internal General
 *  Utility functionality that doesn't fit in any other category.
 */

/** @defgroup Memory-Internal Memory
 *  Allocators, deallocators and memory manipulation.
 */

/** @defgroup Platform-Utility-Internal Platform
 *  Platform specific functionality.
 */

/** @defgroup RTTI-Internal RTTI
 *  Run-time type information defining and querying.
 */

/** @defgroup Serialization-Internal Serialization
 *  Serialization and deserialization of native objects.
 */

/** @defgroup String-Internal String
 *  String manipulation.
 */

/** @defgroup Threading-Internal Threading
 *  Thread manipulation and synchronization.
 */

/** @} */
/** @} */

/** @defgroup Plugins Plugins
 *	Reference documentation for implementations of various plugins, useful primarily to those extending the engine.
 */

/** @defgroup Implementation-Internal Implementation
 *	Very specialized base classes, templates and helper code used for construction of more concrete types.
 */

#define B3D_PROFILING_ENABLED 1
#define B3D_CONCAT_(a, b) a##b
#define B3D_CONCAT(a, b) B3D_CONCAT_(a, b)

// Config from the build system
#include "B3DFrameworkConfig.h"

// Platform-specific stuff
#include "Prerequisites/B3DPlatformDefines.h"

#if B3D_COMPILER_MSVC

// TODO - This is not deactivated anywhere, therefore it applies to any file that includes this header.
//      - Right now I don't have an easier way to apply these warnings globally so I'm keeping it this way.

// Secure versions aren't multiplatform, so we won't be using them
#	if !defined(_CRT_SECURE_NO_WARNINGS)
#		define _CRT_SECURE_NO_WARNINGS
#	endif

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#	pragma warning(disable : 4251)

// disable: 'X' Function call with parameters that may be unsafe
#	pragma warning(disable : 4996)

// disable: decorated name length exceeded, name was truncated
// Happens with really long type names. Even fairly standard use
// of std::unordered_map with custom parameters, meaning I can't
// really do much to avoid it. It shouldn't effect execution
// but might cause problems if you compile library
// with one compiler and use it in another.
#	pragma warning(disable : 4503)

// disable: C++ exception handler used, but unwind semantics are not enabled
// We don't care about this as any exception is meant to crash the program.
#	pragma warning(disable : 4530)

#endif

// Windows Settings
#if B3D_PLATFORM_WIN32
// Win32 compilers use _DEBUG for specifying debug builds.
// for MinGW, we set DEBUG
#	if defined(_DEBUG) || defined(DEBUG)
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#endif

// Linux/Apple Settings
#if B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
// A quick define to overcome different names for the same function
#	define stricmp strcasecmp

#	ifdef DEBUG
#		define B3D_DEBUG 1
#	else
#		define B3D_DEBUG 0
#	endif

#endif

#if B3D_DEBUG
#	define B3D_DEBUG_ONLY(x) x
#	define B3D_ASSERT(x) assert(x)
#	define B3D_CHECK(x) (B3D_LIKELY(!!(x)) || ([]() { assert(false); }(), false))
#	define B3D_CHECK_LOG(x, message, ...) (B3D_LIKELY(!!(x)) || ([]() { assert(false && message); }(), false))
#else
#	define B3D_DEBUG_ONLY(x)
#	define B3D_ASSERT(x)
#	define B3D_CHECK(x) (!!(x))
#	define B3D_CHECK_LOG(x, ...) (!!(x))
#endif

// Short-hand names for various built-in types
#include "Prerequisites/B3DTypes.h"

#include "Allocators/B3DMemoryAllocator.h"

// Common threading functionality
#include "Threading/B3DThreading.h"

// Commonly used standard headers
#include "Prerequisites/B3DStdHeaders.h"

// Forward declarations
#include "Prerequisites/B3DFwdDeclUtil.h"

#include "String/B3DString.h"
#include "Utility/B3DMessageHandlerFwd.h"
#include "Utility/B3DFlags.h"
#include "Utility/B3DUtil.h"
#include "Utility/B3DShared.h"
#include "Utility/B3DEvent.h"
#include "Utility/B3DPlatformUtility.h"
#include "Utility/B3DNonCopyable.h"
#include "Utility/B3DTArray.h"
#include "Utility/B3DTArrayView.h"
#include "FileSystem/B3DPath.h"
#include "Error/B3DCrashHandler.h"
