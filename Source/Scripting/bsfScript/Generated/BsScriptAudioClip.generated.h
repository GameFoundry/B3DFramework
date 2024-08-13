//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs { class AudioClip; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioClip : public TScriptResource<ScriptAudioClip, AudioClip>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AudioClip")

		ScriptAudioClip(MonoObject* managedInstance, const TResourceHandle<AudioClip>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptAudioClip* self);

		static uint32_t InternalGetBitDepth(ScriptAudioClip* self);
		static uint32_t InternalGetFrequency(ScriptAudioClip* self);
		static uint32_t InternalGetNumChannels(ScriptAudioClip* self);
		static AudioFormat InternalGetFormat(ScriptAudioClip* self);
		static AudioReadMode InternalGetReadMode(ScriptAudioClip* self);
		static float InternalGetLength(ScriptAudioClip* self);
		static uint32_t InternalGetNumSamples(ScriptAudioClip* self);
		static bool InternalIs3D(ScriptAudioClip* self);
	};
}
