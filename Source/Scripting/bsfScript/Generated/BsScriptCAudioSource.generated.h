//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioSource.h"

namespace bs { class CAudioSource; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioSource : public TScriptComponent<ScriptAudioSource, CAudioSource>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AudioSource")

		ScriptAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value);

	private:
		static void InternalSetClip(ScriptAudioSource* thisPtr, MonoObject* clip);
		static MonoObject* InternalGetClip(ScriptAudioSource* thisPtr);
		static void InternalSetVolume(ScriptAudioSource* thisPtr, float volume);
		static float InternalGetVolume(ScriptAudioSource* thisPtr);
		static void InternalSetPitch(ScriptAudioSource* thisPtr, float pitch);
		static float InternalGetPitch(ScriptAudioSource* thisPtr);
		static void InternalSetIsLooping(ScriptAudioSource* thisPtr, bool loop);
		static bool InternalGetIsLooping(ScriptAudioSource* thisPtr);
		static void InternalSetPriority(ScriptAudioSource* thisPtr, uint32_t priority);
		static uint32_t InternalGetPriority(ScriptAudioSource* thisPtr);
		static void InternalSetMinDistance(ScriptAudioSource* thisPtr, float distance);
		static float InternalGetMinDistance(ScriptAudioSource* thisPtr);
		static void InternalSetAttenuation(ScriptAudioSource* thisPtr, float attenuation);
		static float InternalGetAttenuation(ScriptAudioSource* thisPtr);
		static void InternalSetTime(ScriptAudioSource* thisPtr, float time);
		static float InternalGetTime(ScriptAudioSource* thisPtr);
		static void InternalSetPlayOnStart(ScriptAudioSource* thisPtr, bool enable);
		static bool InternalGetPlayOnStart(ScriptAudioSource* thisPtr);
		static void InternalPlay(ScriptAudioSource* thisPtr);
		static void InternalPause(ScriptAudioSource* thisPtr);
		static void InternalStop(ScriptAudioSource* thisPtr);
		static AudioSourceState InternalGetState(ScriptAudioSource* thisPtr);
	};
}
