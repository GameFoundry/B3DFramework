//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClipImportOptions.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"
#include "../../../Foundation/bsfCore/Audio/BsAudioClip.h"

namespace bs { class AudioClipImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioClipImportOptions : public TScriptReflectable<ScriptAudioClipImportOptions, AudioClipImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AudioClipImportOptions")

		ScriptAudioClipImportOptions(MonoObject* managedInstance, const SPtr<AudioClipImportOptions>& value);

		static MonoObject* Create(const SPtr<AudioClipImportOptions>& value);

	private:
		static AudioFormat InternalGetFormat(ScriptAudioClipImportOptions* thisPtr);
		static void InternalSetFormat(ScriptAudioClipImportOptions* thisPtr, AudioFormat value);
		static AudioReadMode InternalGetReadMode(ScriptAudioClipImportOptions* thisPtr);
		static void InternalSetReadMode(ScriptAudioClipImportOptions* thisPtr, AudioReadMode value);
		static bool InternalGetIs3D(ScriptAudioClipImportOptions* thisPtr);
		static void InternalSetIs3D(ScriptAudioClipImportOptions* thisPtr, bool value);
		static uint32_t InternalGetBitDepth(ScriptAudioClipImportOptions* thisPtr);
		static void InternalSetBitDepth(ScriptAudioClipImportOptions* thisPtr, uint32_t value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
