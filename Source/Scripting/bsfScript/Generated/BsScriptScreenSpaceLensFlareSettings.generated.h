//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ScreenSpaceLensFlareSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptScreenSpaceLensFlareSettings : public TScriptReflectable<ScriptScreenSpaceLensFlareSettings, ScreenSpaceLensFlareSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ScreenSpaceLensFlareSettings")

		ScriptScreenSpaceLensFlareSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceLensFlareSettings>& value);

		static MonoObject* create(const SPtr<ScreenSpaceLensFlareSettings>& value);

	private:
		static void Internal_ScreenSpaceLensFlareSettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setenabled(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
		static uint32_t Internal_getdownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setdownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value);
		static float Internal_getthreshold(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setthreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static uint32_t Internal_getghostCount(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setghostCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value);
		static float Internal_getghostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setghostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float Internal_getbrightness(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setbrightness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float Internal_getfilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setfilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static bool Internal_gethalo(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_sethalo(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
		static float Internal_gethaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_sethaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float Internal_gethaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_sethaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float Internal_gethaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_sethaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static float Internal_gethaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_sethaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static bool Internal_getchromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setchromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
		static float Internal_getchromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setchromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr, float value);
		static bool Internal_getbicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr);
		static void Internal_setbicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value);
	};
}
