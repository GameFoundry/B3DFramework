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
		static float InternalGetFilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr, float value);
		static float InternalGetFilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr, float value);
		static float InternalGetFilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr, float value);
		static float InternalGetFilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr, float value);
		static float InternalGetFilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr, float value);
		static float InternalGetFilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr, float value);
		static float InternalGetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr);
		static void InternalSetFilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr, float value);
	};
}
