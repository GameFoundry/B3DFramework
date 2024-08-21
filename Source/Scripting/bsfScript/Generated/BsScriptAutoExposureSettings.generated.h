//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct AutoExposureSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAutoExposureSettings : public TScriptReflectableWrapper<AutoExposureSettings, ScriptAutoExposureSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "AutoExposureSettings")

		ScriptAutoExposureSettings(const SPtr<AutoExposureSettings>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalAutoExposureSettings(MonoObject* scriptObject);
		static float InternalGetHistogramLog2Min(ScriptAutoExposureSettings* self);
		static void InternalSetHistogramLog2Min(ScriptAutoExposureSettings* self, float value);
		static float InternalGetHistogramLog2Max(ScriptAutoExposureSettings* self);
		static void InternalSetHistogramLog2Max(ScriptAutoExposureSettings* self, float value);
		static float InternalGetHistogramPctLow(ScriptAutoExposureSettings* self);
		static void InternalSetHistogramPctLow(ScriptAutoExposureSettings* self, float value);
		static float InternalGetHistogramPctHigh(ScriptAutoExposureSettings* self);
		static void InternalSetHistogramPctHigh(ScriptAutoExposureSettings* self, float value);
		static float InternalGetMinEyeAdaptation(ScriptAutoExposureSettings* self);
		static void InternalSetMinEyeAdaptation(ScriptAutoExposureSettings* self, float value);
		static float InternalGetMaxEyeAdaptation(ScriptAutoExposureSettings* self);
		static void InternalSetMaxEyeAdaptation(ScriptAutoExposureSettings* self, float value);
		static float InternalGetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* self);
		static void InternalSetEyeAdaptationSpeedUp(ScriptAutoExposureSettings* self, float value);
		static float InternalGetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* self);
		static void InternalSetEyeAdaptationSpeedDown(ScriptAutoExposureSettings* self, float value);
	};
}
