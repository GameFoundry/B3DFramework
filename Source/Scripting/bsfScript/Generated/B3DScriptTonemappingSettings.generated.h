//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct TonemappingSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTonemappingSettings : public TScriptReflectableWrapper<TonemappingSettings, ScriptTonemappingSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TonemappingSettings")

		ScriptTonemappingSettings(const TShared<TonemappingSettings>& nativeObject);
		~ScriptTonemappingSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTonemappingSettings(MonoObject* scriptObject);
		static float InternalGetFilmicCurveShoulderStrength(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveShoulderStrength(ScriptTonemappingSettings* self, float value);
		static float InternalGetFilmicCurveLinearStrength(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveLinearStrength(ScriptTonemappingSettings* self, float value);
		static float InternalGetFilmicCurveLinearAngle(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveLinearAngle(ScriptTonemappingSettings* self, float value);
		static float InternalGetFilmicCurveToeStrength(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveToeStrength(ScriptTonemappingSettings* self, float value);
		static float InternalGetFilmicCurveToeNumerator(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveToeNumerator(ScriptTonemappingSettings* self, float value);
		static float InternalGetFilmicCurveToeDenominator(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveToeDenominator(ScriptTonemappingSettings* self, float value);
		static float InternalGetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* self);
		static void InternalSetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* self, float value);
	};
}
