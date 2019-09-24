//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Math/BsVector2.h"

namespace bs { struct DepthOfFieldSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptDepthOfFieldSettings : public TScriptReflectable<ScriptDepthOfFieldSettings, DepthOfFieldSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "DepthOfFieldSettings")

		ScriptDepthOfFieldSettings(MonoObject* managedInstance, const SPtr<DepthOfFieldSettings>& value);

		static MonoObject* create(const SPtr<DepthOfFieldSettings>& value);

	private:
		static void Internal_DepthOfFieldSettings(MonoObject* managedInstance);
		static MonoObject* Internal_getbokehShape(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setbokehShape(ScriptDepthOfFieldSettings* thisPtr, MonoObject* value);
		static bool Internal_getenabled(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setenabled(ScriptDepthOfFieldSettings* thisPtr, bool value);
		static DepthOfFieldType Internal_gettype(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_settype(ScriptDepthOfFieldSettings* thisPtr, DepthOfFieldType value);
		static float Internal_getfocalDistance(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setfocalDistance(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getfocalRange(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setfocalRange(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getnearTransitionRange(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setnearTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getfarTransitionRange(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setfarTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getnearBlurAmount(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setnearBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getfarBlurAmount(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setfarBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getmaxBokehSize(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setmaxBokehSize(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getadaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setadaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getadaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setadaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getapertureSize(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setapertureSize(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float Internal_getfocalLength(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setfocalLength(ScriptDepthOfFieldSettings* thisPtr, float value);
		static void Internal_getsensorSize(ScriptDepthOfFieldSettings* thisPtr, Vector2* __output);
		static void Internal_setsensorSize(ScriptDepthOfFieldSettings* thisPtr, Vector2* value);
		static bool Internal_getbokehOcclusion(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setbokehOcclusion(ScriptDepthOfFieldSettings* thisPtr, bool value);
		static float Internal_getocclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr);
		static void Internal_setocclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr, float value);
	};
}
