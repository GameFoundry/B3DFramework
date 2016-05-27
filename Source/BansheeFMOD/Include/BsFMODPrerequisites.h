//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_FMOD_EXPORTS
#		define BS_FMOD_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_FMOD_EXPORT
#       else
#    		define BS_FMOD_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined (BS_GCC_VISIBILITY)
#    define BS_FMOD_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_FMOD_EXPORT
#endif

namespace BansheeEngine
{
	/** Contains data describing an audio file. */
	struct AudioFileInfo
	{
		UINT32 numSamples; /**< Total number of audio samples in the audio data (includes all channels). */
		UINT32 sampleRate; /**< Number of audio samples per second, per channel. */
		UINT32 numChannels; /**< Number of channels. Each channel has its own set of samples. */
		UINT32 bitDepth; /**< Number of bits per sample. */
	};
}

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup FMOD BansheeFMOD
 *	Audio system implementation using FMOD.
 */

/** @} */