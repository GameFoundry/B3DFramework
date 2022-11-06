//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct AutoExposureSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAutoExposureSettings : public TScriptReflectable<ScriptAutoExposureSettings, AutoExposureSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AutoExposureSettings")

		ScriptAutoExposureSettings(MonoObject* managedInstance, const SPtr<AutoExposureSettings>& value);

		static MonoObject* Create(const SPtr<AutoExposureSettings>& value);

	private:
		static void InternalAutoExposureSettings(MonoObject* managedInstance);
		static float InternalGetHistogramLog2Min(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetHistogramLog2Min(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetHistogramLog2Max(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetHistogramLog2Max(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetHistogramPctLow(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetHistogramPctLow(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetHistogramPctHigh(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetHistogramPctHigh(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetMinEyeAdaptation(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetMinEyeAdaptation(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetMaxEyeAdaptation(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetMaxEyeAdaptation(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* thisPtr, float value);
		static float InternalGetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* thisPtr);
		static void InternalSetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* thisPtr, float value);
	};
}
