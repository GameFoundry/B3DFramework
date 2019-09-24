//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct TonemappingSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTonemappingSettings : public TScriptReflectable<ScriptTonemappingSettings, TonemappingSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "TonemappingSettings")

		ScriptTonemappingSettings(MonoObject* managedInstance, const SPtr<TonemappingSettings>& value);

		static MonoObject* create(const SPtr<TonemappingSettings>& value);

	private:
		static void Internal_TonemappingSettings(MonoObject* managedInstance);
		static float Internal_getfilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveShoulderStrength(ScriptTonemappingSettings* thisPtr, float value);
		static float Internal_getfilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveLinearStrength(ScriptTonemappingSettings* thisPtr, float value);
		static float Internal_getfilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveLinearAngle(ScriptTonemappingSettings* thisPtr, float value);
		static float Internal_getfilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveToeStrength(ScriptTonemappingSettings* thisPtr, float value);
		static float Internal_getfilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveToeNumerator(ScriptTonemappingSettings* thisPtr, float value);
		static float Internal_getfilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveToeDenominator(ScriptTonemappingSettings* thisPtr, float value);
		static float Internal_getfilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr);
		static void Internal_setfilmicCurveLinearWhitePoint(ScriptTonemappingSettings* thisPtr, float value);
	};
}
