//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Image/BsColor.h"

namespace bs
{
	struct BloomSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptBloomSettings : public TScriptReflectable<ScriptBloomSettings, BloomSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "BloomSettings")

		ScriptBloomSettings(MonoObject* managedInstance, const SPtr<BloomSettings>& value);

		static MonoObject* Create(const SPtr<BloomSettings>& value);

	private:
		static void InternalBloomSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptBloomSettings* thisPtr);
		static void InternalSetEnabled(ScriptBloomSettings* thisPtr, bool value);
		static uint32_t InternalGetQuality(ScriptBloomSettings* thisPtr);
		static void InternalSetQuality(ScriptBloomSettings* thisPtr, uint32_t value);
		static float InternalGetThreshold(ScriptBloomSettings* thisPtr);
		static void InternalSetThreshold(ScriptBloomSettings* thisPtr, float value);
		static float InternalGetIntensity(ScriptBloomSettings* thisPtr);
		static void InternalSetIntensity(ScriptBloomSettings* thisPtr, float value);
		static void InternalGetTint(ScriptBloomSettings* thisPtr, Color* __output);
		static void InternalSetTint(ScriptBloomSettings* thisPtr, Color* value);
		static float InternalGetFilterSize(ScriptBloomSettings* thisPtr);
		static void InternalSetFilterSize(ScriptBloomSettings* thisPtr, float value);
	};
} // namespace bs
