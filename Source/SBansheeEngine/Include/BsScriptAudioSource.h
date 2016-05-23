//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for AudioSource. */
	class BS_SCR_BE_EXPORT ScriptAudioSource : public ScriptObject<ScriptAudioSource>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeAudioSource")

	private:
		ScriptAudioSource(MonoObject* instance, const SPtr<AudioSource>& source);

		SPtr<AudioSource> mSource;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_Destroy(ScriptAudioSource* thisPtr);

		static void internal_SetPosition(ScriptAudioSource* thisPtr, Vector3* position);
		static void internal_SetVelocity(ScriptAudioSource* thisPtr, Vector3* velocity);
		static void internal_SetClip(ScriptAudioSource* thisPtr, ScriptAudioClip* clip);
		static void internal_SetVolume(ScriptAudioSource* thisPtr, float volume);
		static void internal_SetPitch(ScriptAudioSource* thisPtr, float pitch);
		static void internal_SetIsLooping(ScriptAudioSource* thisPtr, bool loop);
		static void internal_SetPriority(ScriptAudioSource* thisPtr, UINT32 priority);
		static void internal_SetMinDistance(ScriptAudioSource* thisPtr, float distance);
		static void internal_SetAttenuation(ScriptAudioSource* thisPtr, float attenuation);
		static void internal_Play(ScriptAudioSource* thisPtr);
		static void internal_Pause(ScriptAudioSource* thisPtr);
		static void internal_Stop(ScriptAudioSource* thisPtr);
		static void internal_Seek(ScriptAudioSource* thisPtr, float position);
		static UINT32 internal_GetState(ScriptAudioSource* thisPtr);
	};

	/** @} */
}