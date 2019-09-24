//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct AutoExposureSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAutoExposureSettings : public TScriptReflectable<ScriptAutoExposureSettings, AutoExposureSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AutoExposureSettings")

		ScriptAutoExposureSettings(MonoObject* managedInstance, const SPtr<AutoExposureSettings>& value);

		static MonoObject* create(const SPtr<AutoExposureSettings>& value);

	private:
		static void Internal_AutoExposureSettings(MonoObject* managedInstance);
		static float Internal_gethistogramLog2Min(ScriptAutoExposureSettings* thisPtr);
		static void Internal_sethistogramLog2Min(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_gethistogramLog2Max(ScriptAutoExposureSettings* thisPtr);
		static void Internal_sethistogramLog2Max(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_gethistogramPctLow(ScriptAutoExposureSettings* thisPtr);
		static void Internal_sethistogramPctLow(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_gethistogramPctHigh(ScriptAutoExposureSettings* thisPtr);
		static void Internal_sethistogramPctHigh(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_getminEyeAdaptation(ScriptAutoExposureSettings* thisPtr);
		static void Internal_setminEyeAdaptation(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_getmaxEyeAdaptation(ScriptAutoExposureSettings* thisPtr);
		static void Internal_setmaxEyeAdaptation(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_geteyeAdaptationSpeedUp(ScriptAutoExposureSettings* thisPtr);
		static void Internal_seteyeAdaptationSpeedUp(ScriptAutoExposureSettings* thisPtr, float value);
		static float Internal_geteyeAdaptationSpeedDown(ScriptAutoExposureSettings* thisPtr);
		static void Internal_seteyeAdaptationSpeedDown(ScriptAutoExposureSettings* thisPtr, float value);
	};
}
