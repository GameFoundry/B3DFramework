//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioSource.h"

namespace bs { class CAudioSource; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioSource : public TScriptGameObjectWrapper<CAudioSource, ScriptAudioSource>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AudioSource")

		ScriptAudioSource(const GameObjectHandle<CAudioSource>& nativeObject);
		~ScriptAudioSource();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetClip(ScriptAudioSource* self, MonoObject* clip);
		static MonoObject* InternalGetClip(ScriptAudioSource* self);
		static void InternalSetVolume(ScriptAudioSource* self, float volume);
		static float InternalGetVolume(ScriptAudioSource* self);
		static void InternalSetPitch(ScriptAudioSource* self, float pitch);
		static float InternalGetPitch(ScriptAudioSource* self);
		static void InternalSetIsLooping(ScriptAudioSource* self, bool loop);
		static bool InternalGetIsLooping(ScriptAudioSource* self);
		static void InternalSetPriority(ScriptAudioSource* self, uint32_t priority);
		static uint32_t InternalGetPriority(ScriptAudioSource* self);
		static void InternalSetMinDistance(ScriptAudioSource* self, float distance);
		static float InternalGetMinDistance(ScriptAudioSource* self);
		static void InternalSetAttenuation(ScriptAudioSource* self, float attenuation);
		static float InternalGetAttenuation(ScriptAudioSource* self);
		static void InternalSetTime(ScriptAudioSource* self, float time);
		static float InternalGetTime(ScriptAudioSource* self);
		static void InternalSetPlayOnStart(ScriptAudioSource* self, bool enable);
		static bool InternalGetPlayOnStart(ScriptAudioSource* self);
		static void InternalPlay(ScriptAudioSource* self);
		static void InternalPause(ScriptAudioSource* self);
		static void InternalStop(ScriptAudioSource* self);
		static AudioSourceState InternalGetState(ScriptAudioSource* self);
	};
}
