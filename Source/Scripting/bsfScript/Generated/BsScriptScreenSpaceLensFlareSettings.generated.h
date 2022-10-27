//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs
{
	struct ScreenSpaceLensFlareSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptScreenSpaceLensFlareSettings : public TScriptReflectable<ScriptScreenSpaceLensFlareSettings, ScreenSpaceLensFlareSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ScreenSpaceLensFlareSettings")

		ScriptScreenSpaceLensFlareSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceLensFlareSettings>& value);

		static MonoObject* Create(const SPtr<ScreenSpaceLensFlareSettings>& value);

	private:
		static void InternalScreenSpaceLensFlareSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetEnabled(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
		static uint32_t InternalGetDownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetDownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value);
		static float InternalGetThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static uint32_t InternalGetGhostCount(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetGhostCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value);
		static float InternalGetGhostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetGhostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float InternalGetBrightness(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetBrightness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float InternalGetFilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetFilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static bool InternalGetHalo(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetHalo(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
		static float InternalGetHaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetHaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float InternalGetHaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetHaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float InternalGetHaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetHaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float InternalGetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static bool InternalGetChromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetChromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
		static float InternalGetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static bool InternalGetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void InternalSetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
	};
} // namespace bs
