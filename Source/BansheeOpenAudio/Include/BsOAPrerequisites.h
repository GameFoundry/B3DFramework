//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_OA_EXPORTS
#		define BS_OA_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_OA_EXPORT
#       else
#    		define BS_OA_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_OA_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_OA_EXPORT
#endif

namespace BansheeEngine
{
	class OAAudioListener;
	class OAAudioSource;
}

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup OpenAudio BansheeOpenAudio
 *	Open source audio implementation using OpenAL, libFLAC and libvorbis.
 */

/** @} */