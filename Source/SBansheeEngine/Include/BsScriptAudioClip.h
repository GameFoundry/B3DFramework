//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsAudioClip.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AudioClip. */
	class BS_SCR_BE_EXPORT ScriptAudioClip : public TScriptResource<ScriptAudioClip, AudioClip>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AudioClip")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;

		ScriptAudioClip(MonoObject* instance, const HAudioClip& audioClip);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static UINT32 internal_GetBitDepth(ScriptAudioClip* thisPtr);
		static UINT32 internal_GetSampleRate(ScriptAudioClip* thisPtr);
		static UINT32 internal_GetNumChannels(ScriptAudioClip* thisPtr);
		static AudioFormat internal_GetAudioFormat(ScriptAudioClip* thisPtr);
		static AudioReadMode internal_GetReadMode(ScriptAudioClip* thisPtr);
		static UINT32 internal_GetNumSamples(ScriptAudioClip* thisPtr);
		static float internal_GetDuration(ScriptAudioClip* thisPtr);
		static bool internal_GetIs3D(ScriptAudioClip* thisPtr);
	};

	/** @} */
}