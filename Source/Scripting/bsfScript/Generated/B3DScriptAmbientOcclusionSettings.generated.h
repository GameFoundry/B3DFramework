//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct AmbientOcclusionSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAmbientOcclusionSettings : public TScriptReflectableWrapper<AmbientOcclusionSettings, ScriptAmbientOcclusionSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AmbientOcclusionSettings")

		ScriptAmbientOcclusionSettings(const TShared<AmbientOcclusionSettings>& nativeObject);
		~ScriptAmbientOcclusionSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalAmbientOcclusionSettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptAmbientOcclusionSettings* self);
		static void InternalSetEnabled(ScriptAmbientOcclusionSettings* self, bool value);
		static float InternalGetRadius(ScriptAmbientOcclusionSettings* self);
		static void InternalSetRadius(ScriptAmbientOcclusionSettings* self, float value);
		static float InternalGetBias(ScriptAmbientOcclusionSettings* self);
		static void InternalSetBias(ScriptAmbientOcclusionSettings* self, float value);
		static float InternalGetFadeDistance(ScriptAmbientOcclusionSettings* self);
		static void InternalSetFadeDistance(ScriptAmbientOcclusionSettings* self, float value);
		static float InternalGetFadeRange(ScriptAmbientOcclusionSettings* self);
		static void InternalSetFadeRange(ScriptAmbientOcclusionSettings* self, float value);
		static float InternalGetIntensity(ScriptAmbientOcclusionSettings* self);
		static void InternalSetIntensity(ScriptAmbientOcclusionSettings* self, float value);
		static float InternalGetPower(ScriptAmbientOcclusionSettings* self);
		static void InternalSetPower(ScriptAmbientOcclusionSettings* self, float value);
		static uint32_t InternalGetQuality(ScriptAmbientOcclusionSettings* self);
		static void InternalSetQuality(ScriptAmbientOcclusionSettings* self, uint32_t value);
	};
}
