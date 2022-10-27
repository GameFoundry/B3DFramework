//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Math/BsVector2.h"

namespace bs
{
	struct DepthOfFieldSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptDepthOfFieldSettings : public TScriptReflectable<ScriptDepthOfFieldSettings, DepthOfFieldSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "DepthOfFieldSettings")

		ScriptDepthOfFieldSettings(MonoObject* managedInstance, const SPtr<DepthOfFieldSettings>& value);

		static MonoObject* Create(const SPtr<DepthOfFieldSettings>& value);

	private:
		static void InternalDepthOfFieldSettings(MonoObject* managedInstance);
		static MonoObject* InternalGetBokehShape(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetBokehShape(ScriptDepthOfFieldSettings* thisPtr, MonoObject* value);
		static bool InternalGetEnabled(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetEnabled(ScriptDepthOfFieldSettings* thisPtr, bool value);
		static DepthOfFieldType InternalGetType(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetType(ScriptDepthOfFieldSettings* thisPtr, DepthOfFieldType value);
		static float InternalGetFocalDistance(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetFocalDistance(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetFocalRange(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetFocalRange(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetNearTransitionRange(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetNearTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetFarTransitionRange(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetFarTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetNearBlurAmount(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetNearBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetFarBlurAmount(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetFarBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetMaxBokehSize(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetMaxBokehSize(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetApertureSize(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetApertureSize(ScriptDepthOfFieldSettings* thisPtr, float value);
		static float InternalGetFocalLength(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetFocalLength(ScriptDepthOfFieldSettings* thisPtr, float value);
		static void InternalGetSensorSize(ScriptDepthOfFieldSettings* thisPtr, Vector2* __output);
		static void InternalSetSensorSize(ScriptDepthOfFieldSettings* thisPtr, Vector2* value);
		static bool InternalGetBokehOcclusion(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetBokehOcclusion(ScriptDepthOfFieldSettings* thisPtr, bool value);
		static float InternalGetOcclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr);
		static void InternalSetOcclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr, float value);
	};
} // namespace bs
