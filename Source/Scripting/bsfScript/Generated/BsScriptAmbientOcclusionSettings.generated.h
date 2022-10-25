//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs
{
	struct AmbientOcclusionSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAmbientOcclusionSettings : public TScriptReflectable<ScriptAmbientOcclusionSettings, AmbientOcclusionSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AmbientOcclusionSettings")

		ScriptAmbientOcclusionSettings(MonoObject* managedInstance, const SPtr<AmbientOcclusionSettings>& value);

		static MonoObject* Create(const SPtr<AmbientOcclusionSettings>& value);

	private:
		static void InternalAmbientOcclusionSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetEnabled(ScriptAmbientOcclusionSettings* thisPtr, bool value);
		static float InternalGetRadius(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetRadius(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float InternalGetBias(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetBias(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float InternalGetFadeDistance(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetFadeDistance(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float InternalGetFadeRange(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetFadeRange(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float InternalGetIntensity(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetIntensity(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float InternalGetPower(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetPower(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static uint32_t InternalGetQuality(ScriptAmbientOcclusionSettings* thisPtr);
		static void InternalSetQuality(ScriptAmbientOcclusionSettings* thisPtr, uint32_t value);
	};
} // namespace bs
