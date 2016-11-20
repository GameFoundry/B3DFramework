//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_SL_EXPORTS
#		define BS_SL_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_SL_EXPORT
#       else
#    		define BS_SL_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_SL_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_SL_EXPORTS
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup BansheeSL BansheeSL
 *	Implementation of the Banshee Shading Language.
 */

/** @} */

namespace bs
{
	extern const char* SystemName;
}