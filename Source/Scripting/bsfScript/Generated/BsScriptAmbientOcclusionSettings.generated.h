//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct AmbientOcclusionSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAmbientOcclusionSettings : public TScriptReflectableWrapper<AmbientOcclusionSettings, ScriptAmbientOcclusionSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "AmbientOcclusionSettings")

		ScriptAmbientOcclusionSettings(const SPtr<AmbientOcclusionSettings>& nativeObject, MonoObject* scriptObject);

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
