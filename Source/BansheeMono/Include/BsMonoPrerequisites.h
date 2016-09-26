//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_MONO_EXPORTS
#		define BS_MONO_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_MONO_EXPORT
#       else
#    		define BS_MONO_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_MONO_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_MONO_EXPORT
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup Mono BansheeMono
 *	Scripting language backend using Mono's CLR.
 */

/** @} */

namespace BansheeEngine
{
	class MonoManager;
	class MonoAssembly;
	class MonoClass;
	class MonoMethod;
	class MonoField;
	class MonoProperty;

	/** A list of all valid Mono primitive types. */
	enum class MonoPrimitiveType
	{
		Boolean,
		Char,
		I8,
		U8,
		I16,
		U16,
		I32,
		U32,
		I64,
		U64,
		R32,
		R64,
		String,
		ValueType,
		Class,
		Array,
		Generic,
		Unknown
	};

	/**	Returns the level of member visibility in the class. */
	enum class MonoMemberVisibility // Note: Elements must be ordered from most to least hidden
	{
		Private,
		Protected,
		Internal,
		ProtectedInternal,
		Public
	};
}

typedef struct _MonoClass MonoClass;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoImage MonoImage;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoProperty MonoProperty;
typedef struct _MonoObject MonoObject;
typedef struct _MonoString MonoString;
typedef struct _MonoArray MonoArray;
typedef struct _MonoReflectionType MonoReflectionType;
typedef struct _MonoException MonoException;
typedef struct _MonoClassField MonoClassField;