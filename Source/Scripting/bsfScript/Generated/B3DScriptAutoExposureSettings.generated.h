//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct AutoExposureSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAutoExposureSettings : public TScriptReflectableWrapper<AutoExposureSettings, ScriptAutoExposureSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AutoExposureSettings")

		ScriptAutoExposureSettings(const TShared<AutoExposureSettings>& nativeObject);
		~ScriptAutoExposureSettings();

		static void SetupScriptBindings();

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
