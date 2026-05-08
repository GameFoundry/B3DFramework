//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Utility/Math/B3DVector2.h"

namespace b3d { struct DepthOfFieldSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDepthOfFieldSettings : public TScriptReflectableWrapper<DepthOfFieldSettings, ScriptDepthOfFieldSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "DepthOfFieldSettings")

		ScriptDepthOfFieldSettings(const TShared<DepthOfFieldSettings>& nativeObject);
		~ScriptDepthOfFieldSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalDepthOfFieldSettings(MonoObject* scriptObject);
		static MonoObject* InternalGetBokehShape(ScriptDepthOfFieldSettings* self);
		static void InternalSetBokehShape(ScriptDepthOfFieldSettings* self, MonoObject* value);
		static bool InternalGetEnabled(ScriptDepthOfFieldSettings* self);
		static void InternalSetEnabled(ScriptDepthOfFieldSettings* self, bool value);
		static DepthOfFieldType InternalGetType(ScriptDepthOfFieldSettings* self);
		static void InternalSetType(ScriptDepthOfFieldSettings* self, DepthOfFieldType value);
		static float InternalGetFocalDistance(ScriptDepthOfFieldSettings* self);
		static void InternalSetFocalDistance(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetFocalRange(ScriptDepthOfFieldSettings* self);
		static void InternalSetFocalRange(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetNearTransitionRange(ScriptDepthOfFieldSettings* self);
		static void InternalSetNearTransitionRange(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetFarTransitionRange(ScriptDepthOfFieldSettings* self);
		static void InternalSetFarTransitionRange(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetNearBlurAmount(ScriptDepthOfFieldSettings* self);
		static void InternalSetNearBlurAmount(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetFarBlurAmount(ScriptDepthOfFieldSettings* self);
		static void InternalSetFarBlurAmount(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetMaxBokehSize(ScriptDepthOfFieldSettings* self);
		static void InternalSetMaxBokehSize(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* self);
		static void InternalSetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* self);
		static void InternalSetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetApertureSize(ScriptDepthOfFieldSettings* self);
		static void InternalSetApertureSize(ScriptDepthOfFieldSettings* self, float value);
		static float InternalGetFocalLength(ScriptDepthOfFieldSettings* self);
		static void InternalSetFocalLength(ScriptDepthOfFieldSettings* self, float value);
		static void InternalGetSensorSize(ScriptDepthOfFieldSettings* self, TVector2<float>* __output);
		static void InternalSetSensorSize(ScriptDepthOfFieldSettings* self, TVector2<float>* value);
		static bool InternalGetBokehOcclusion(ScriptDepthOfFieldSettings* self);
		static void InternalSetBokehOcclusion(ScriptDepthOfFieldSettings* self, bool value);
		static float InternalGetOcclusionDepthRange(ScriptDepthOfFieldSettings* self);
		static void InternalSetOcclusionDepthRange(ScriptDepthOfFieldSettings* self, float value);
	};
}
