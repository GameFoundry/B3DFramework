//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <assert.h>

/** @defgroup Layers Layers 
 *	User facing API for the engine core, categorized per layer. 
 *  @{
 */

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

/** @} */
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

/** @defgroup Implementation [IMPLEMENTATION]
 *	Very specialized base classes, templates and helper code used for construction of more concrete types.
 */

// 0 - No thread support
// 1 - Render system is thread safe (TODO: NOT WORKING and will probably be removed)
// 2 - Thread support but render system can only be accessed from main thread
#define BS_THREAD_SUPPORT 2

#define BS_PROFILING_ENABLED 1

// Config from the build system
#include "BsFrameworkConfig.h"

// Platform-specific stuff
#include "BsPlatformDefines.h"

#if BS_COMPILER == BS_COMPILER_MSVC

// TODO - This is not deactivated anywhere, therefore it applies to any file that includes this header.
//      - Right now I don't have an easier way to apply these warnings globally so I'm keeping it this way.

// Secure versions aren't multiplatform, so we won't be using them
#if !defined(_CRT_SECURE_NO_WARNINGS)
	#define _CRT_SECURE_NO_WARNINGS
#endif

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#   pragma warning (disable: 4251)

// disable: 'X' Function call with parameters that may be unsafe
#	pragma warning(disable: 4996) 

// disable: decorated name length exceeded, name was truncated
// Happens with really long type names. Even fairly standard use
// of std::unordered_map with custom parameters, meaning I can't
// really do much to avoid it. It shouldn't effect execution
// but might cause problems if you compile library
// with one compiler and use it in another.
#	pragma warning(disable: 4503)

// disable: C++ exception handler used, but unwind semantics are not enabled
// We don't care about this as any exception is meant to crash the program.
#	pragma warning(disable: 4530)

#endif

// Windows Settings
#if BS_PLATFORM == BS_PLATFORM_WIN32
// Win32 compilers use _DEBUG for specifying debug builds.
// for MinGW, we set DEBUG
#   if defined(_DEBUG) || defined(DEBUG)
#       define BS_DEBUG_MODE 1
#   else
#       define BS_DEBUG_MODE 0
#   endif

#endif

// Linux/Apple Settings
#if BS_PLATFORM == BS_PLATFORM_LINUX || BS_PLATFORM == BS_PLATFORM_OSX
// A quick define to overcome different names for the same function
#   define stricmp strcasecmp

#   ifdef DEBUG
#       define BS_DEBUG_MODE 1
#   else
#       define BS_DEBUG_MODE 0
#   endif

#endif

#if BS_DEBUG_MODE
#define BS_DEBUG_ONLY(x) x
#define BS_ASSERT(x) assert(x)
#else
#define BS_DEBUG_ONLY(x)
#define BS_ASSERT(x)
#endif

// Script binding defines

/** 
 * @page scriptBindingMacro Script binding exports
 * 
 * Marks the specific type or a method to be exported to the scripting API. Supports a variety of options which can
 * be specified in the "option:value" format, where multiple options are separated by commas, with no whitespace.
 *
 * Supported options:
 *  - n - Specify a different name for the type in the scripting API (e.g. "n:MyName"). Usable on types and methods.
 *  - v - Specify a different visibility (default is public). Supported values are "public" and "internal". Usable on types
 *		  and methods.
 *  - f - Specify the name of the output file(s) for the script object and its potential wrappers. If not specified
 *		  the name of the type will be used for the file. Usable on types only.
 *	- pl - Specify whether the type is plain or not (default is false). Supported values are "true" or "false". Plain 
 *		  types don't have script interop objects generated, instead their are generated in script code as plain data
 *		  types. No methods are exposed, but all data members and constructors are copied. Usable on types only.
 *	- e - Specify that a method is external and is to be appended to some script class. Such methods must be static
 *		  and as the first parameter accept the instance of the class they operate on. Value of this option should be
 *		  the name of the class to attach this method to. Methods with this parameter must also be part of a class
 *		  with this option. Usable on types and methods.
 *	- ec - Similar to "e", but specifies an external constructor. Such method must have a return value that returns
 *		   an instance of the class its registered for. Value of this option should be the name of the class to attach
 *		   this method to. Methods with this parameter must also be part of a class with the "e" option. Usable on methods
 *		   only.
 *	- pr - Specify the method should be exported as a property in script code. Supported values are "getter" or "setter".
 *		  Getter methods must return a single value and accept no parameters, while setter methods must accept one
 *		  parameter and return no values. Usable on methods only.
 *	- ed - Specify that a type should be exported for use in the editor only. Supported values are "true" or "false".
 *		   Usable on types only.
 */

#if BS_COMPILER == BS_COMPILER_CLANG
	/** @copydoc scriptBindingMacro */
	#define BS_SCRIPT_EXPORT(...) __attribute__((annotate("se," #__VA_ARGS__)))
#else
	/** @copydoc scriptBindingMacro */
	#define BS_SCRIPT_EXPORT(...) 
#endif

// Short-hand names for various built-in types
#include "BsTypes.h"

#include "BsMemoryAllocator.h"

// Useful threading defines
#include "BsThreadDefines.h"

// Commonly used standard headers
#include "BsStdHeaders.h"

// Forward declarations
#include "BsFwdDeclUtil.h"

#include "BsRTTIPrerequisites.h"

#include "BsFlags.h"
#include "BsString.h"
#include "BsMessageHandlerFwd.h"
#include "BsUtil.h"
#include "BsPath.h"
#include "BsStringID.h"
#include "BsEvent.h"
#include "BsPlatformUtility.h"
#include "BsCrashHandler.h"
#include "BsNonCopyable.h"