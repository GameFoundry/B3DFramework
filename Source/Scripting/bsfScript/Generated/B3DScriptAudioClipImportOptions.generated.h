//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptImportOptions.generated.h"
#include "../../../Engine/Core/Audio/B3DAudioClipImportOptions.h"
#include "../../../Engine/Core/Audio/B3DAudioClip.h"
#include "../../../Engine/Core/Audio/B3DAudioClip.h"

namespace b3d { class AudioClipImportOptions; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioClipImportOptions : public TScriptReflectableWrapper<AudioClipImportOptions, ScriptAudioClipImportOptions, ScriptImportOptionsWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AudioClipImportOptions")

		ScriptAudioClipImportOptions(const TShared<AudioClipImportOptions>& nativeObject);
		~ScriptAudioClipImportOptions();

		static void SetupScriptBindings();

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
