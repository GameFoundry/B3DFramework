//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClipImportOptions.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs { class AudioClipImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioClipImportOptions : public TScriptReflectableWrapper<AudioClipImportOptions, ScriptAudioClipImportOptions, ScriptImportOptionsWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "AudioClipImportOptions")

		ScriptAudioClipImportOptions(const SPtr<AudioClipImportOptions>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static AudioFormat InternalGetFormat(ScriptAudioClipImportOptions* self);
		static void InternalSetFormat(ScriptAudioClipImportOptions* self, AudioFormat value);
		static AudioReadMode InternalGetReadMode(ScriptAudioClipImportOptions* self);
		static void InternalSetReadMode(ScriptAudioClipImportOptions* self, AudioReadMode value);
		static bool InternalGetIs3D(ScriptAudioClipImportOptions* self);
		static void InternalSetIs3D(ScriptAudioClipImportOptions* self, bool value);
		static uint32_t InternalGetBitDepth(ScriptAudioClipImportOptions* self);
		static void InternalSetBitDepth(ScriptAudioClipImportOptions* self, uint32_t value);
		static void InternalCreate(MonoObject* scriptObject);
	};
#endif
}
