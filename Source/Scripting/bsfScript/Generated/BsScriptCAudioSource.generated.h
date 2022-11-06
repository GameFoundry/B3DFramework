//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioSource.h"

namespace bs { class CAudioSource; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCAudioSource : public TScriptComponent<ScriptCAudioSource, CAudioSource>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AudioSource")

		ScriptCAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value);

	private:
		static void InternalSetClip(ScriptCAudioSource* thisPtr, MonoObject* clip);
		static MonoObject* InternalGetClip(ScriptCAudioSource* thisPtr);
		static void InternalSetVolume(ScriptCAudioSource* thisPtr, float volume);
		static float InternalGetVolume(ScriptCAudioSource* thisPtr);
		static void InternalSetPitch(ScriptCAudioSource* thisPtr, float pitch);
		static float InternalGetPitch(ScriptCAudioSource* thisPtr);
		static void InternalSetIsLooping(ScriptCAudioSource* thisPtr, bool loop);
		static bool InternalGetIsLooping(ScriptCAudioSource* thisPtr);
		static void InternalSetPriority(ScriptCAudioSource* thisPtr, uint32_t priority);
		static uint32_t InternalGetPriority(ScriptCAudioSource* thisPtr);
		static void InternalSetMinDistance(ScriptCAudioSource* thisPtr, float distance);
		static float InternalGetMinDistance(ScriptCAudioSource* thisPtr);
		static void InternalSetAttenuation(ScriptCAudioSource* thisPtr, float attenuation);
		static float InternalGetAttenuation(ScriptCAudioSource* thisPtr);
		static void InternalSetTime(ScriptCAudioSource* thisPtr, float time);
		static float InternalGetTime(ScriptCAudioSource* thisPtr);
		static void InternalSetPlayOnStart(ScriptCAudioSource* thisPtr, bool enable);
		static bool InternalGetPlayOnStart(ScriptCAudioSource* thisPtr);
		static void InternalPlay(ScriptCAudioSource* thisPtr);
		static void InternalPause(ScriptCAudioSource* thisPtr);
		static void InternalStop(ScriptCAudioSource* thisPtr);
		static AudioSourceState InternalGetState(ScriptCAudioSource* thisPtr);
	};
}
