//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct ScreenSpaceReflectionsSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScreenSpaceReflectionsSettings : public TScriptReflectableWrapper<ScreenSpaceReflectionsSettings, ScriptScreenSpaceReflectionsSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ScreenSpaceReflectionsSettings")

		ScriptScreenSpaceReflectionsSettings(const TShared<ScreenSpaceReflectionsSettings>& nativeObject);
		~ScriptScreenSpaceReflectionsSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalScreenSpaceReflectionsSettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptScreenSpaceReflectionsSettings* self);
		static void InternalSetEnabled(ScriptScreenSpaceReflectionsSettings* self, bool value);
		static uint32_t InternalGetQuality(ScriptScreenSpaceReflectionsSettings* self);
		static void InternalSetQuality(ScriptScreenSpaceReflectionsSettings* self, uint32_t value);
		static float InternalGetIntensity(ScriptScreenSpaceReflectionsSettings* self);
		static void InternalSetIntensity(ScriptScreenSpaceReflectionsSettings* self, float value);
		static float InternalGetMaxRoughness(ScriptScreenSpaceReflectionsSettings* self);
		static void InternalSetMaxRoughness(ScriptScreenSpaceReflectionsSettings* self, float value);
	};
}
