//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioSource.h"

namespace bs { class CAudioSource; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCAudioSource : public TScriptComponent<ScriptCAudioSource, CAudioSource>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AudioSource")

		ScriptCAudioSource(MonoObject* managedInstance, const GameObjectHandle<CAudioSource>& value);

	private:
		static void Internal_setClip(ScriptCAudioSource* thisPtr, MonoObject* clip);
		static MonoObject* Internal_getClip(ScriptCAudioSource* thisPtr);
		static void Internal_setVolume(ScriptCAudioSource* thisPtr, float volume);
		static float Internal_getVolume(ScriptCAudioSource* thisPtr);
		static void Internal_setPitch(ScriptCAudioSource* thisPtr, float pitch);
		static float Internal_getPitch(ScriptCAudioSource* thisPtr);
		static void Internal_setIsLooping(ScriptCAudioSource* thisPtr, bool loop);
		static bool Internal_getIsLooping(ScriptCAudioSource* thisPtr);
		static void Internal_setPriority(ScriptCAudioSource* thisPtr, uint32_t priority);
		static uint32_t Internal_getPriority(ScriptCAudioSource* thisPtr);
		static void Internal_setMinDistance(ScriptCAudioSource* thisPtr, float distance);
		static float Internal_getMinDistance(ScriptCAudioSource* thisPtr);
		static void Internal_setAttenuation(ScriptCAudioSource* thisPtr, float attenuation);
		static float Internal_getAttenuation(ScriptCAudioSource* thisPtr);
		static void Internal_setTime(ScriptCAudioSource* thisPtr, float time);
		static float Internal_getTime(ScriptCAudioSource* thisPtr);
		static void Internal_setPlayOnStart(ScriptCAudioSource* thisPtr, bool enable);
		static bool Internal_getPlayOnStart(ScriptCAudioSource* thisPtr);
		static void Internal_play(ScriptCAudioSource* thisPtr);
		static void Internal_pause(ScriptCAudioSource* thisPtr);
		static void Internal_stop(ScriptCAudioSource* thisPtr);
		static AudioSourceState Internal_getState(ScriptCAudioSource* thisPtr);
	};
}
