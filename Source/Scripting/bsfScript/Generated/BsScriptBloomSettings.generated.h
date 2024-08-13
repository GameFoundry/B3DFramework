//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Image/BsColor.h"

namespace bs { struct BloomSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBloomSettings : public TScriptReflectable<ScriptBloomSettings, BloomSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "BloomSettings")

		ScriptBloomSettings(MonoObject* managedInstance, const SPtr<BloomSettings>& value);

		static MonoObject* Create(const SPtr<BloomSettings>& value);

	private:
		static void InternalBloomSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptBloomSettings* self);
		static void InternalSetEnabled(ScriptBloomSettings* self, bool value);
		static uint32_t InternalGetQuality(ScriptBloomSettings* self);
		static void InternalSetQuality(ScriptBloomSettings* self, uint32_t value);
		static float InternalGetThreshold(ScriptBloomSettings* self);
		static void InternalSetThreshold(ScriptBloomSettings* self, float value);
		static float InternalGetIntensity(ScriptBloomSettings* self);
		static void InternalSetIntensity(ScriptBloomSettings* self, float value);
		static void InternalGetTint(ScriptBloomSettings* self, Color* __output);
		static void InternalSetTint(ScriptBloomSettings* self, Color* value);
		static float InternalGetFilterSize(ScriptBloomSettings* self);
		static void InternalSetFilterSize(ScriptBloomSettings* self, float value);
	};
}
