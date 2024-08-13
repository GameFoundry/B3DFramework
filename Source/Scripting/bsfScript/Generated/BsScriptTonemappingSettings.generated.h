//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct TonemappingSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTonemappingSettings : public TScriptReflectable<ScriptTonemappingSettings, TonemappingSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "TonemappingSettings")

		ScriptTonemappingSettings(MonoObject* managedInstance, const SPtr<TonemappingSettings>& value);

		static MonoObject* Create(const SPtr<TonemappingSettings>& value);

	private:
		static void InternalTonemappingSettings(MonoObject* managedInstance);
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
