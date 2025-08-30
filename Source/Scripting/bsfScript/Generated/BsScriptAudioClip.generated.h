//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace b3d { class AudioClip; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioClip : public TScriptResourceWrapper<AudioClip, ScriptAudioClip>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AudioClip")

		ScriptAudioClip(const TResourceHandle<AudioClip>& nativeObject);
		~ScriptAudioClip();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptAudioClip* self);

		static uint32_t InternalGetBitDepth(ScriptAudioClip* self);
		static uint32_t InternalGetFrequency(ScriptAudioClip* self);
		static uint32_t InternalGetChannelCount(ScriptAudioClip* self);
		static AudioFormat InternalGetFormat(ScriptAudioClip* self);
		static AudioReadMode InternalGetReadMode(ScriptAudioClip* self);
		static float InternalGetLength(ScriptAudioClip* self);
		static uint32_t InternalGetSampleCount(ScriptAudioClip* self);
		static bool InternalIs3D(ScriptAudioClip* self);
	};
}
