//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct ScreenSpaceLensFlareSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScreenSpaceLensFlareSettings : public TScriptReflectableWrapper<ScreenSpaceLensFlareSettings, ScriptScreenSpaceLensFlareSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ScreenSpaceLensFlareSettings")

		ScriptScreenSpaceLensFlareSettings(const TShared<ScreenSpaceLensFlareSettings>& nativeObject);
		~ScriptScreenSpaceLensFlareSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalScreenSpaceLensFlareSettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetEnabled(ScriptScreenSpaceLensFlareSettings* self, bool value);
		static uint32_t InternalGetDownsampleCount(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetDownsampleCount(ScriptScreenSpaceLensFlareSettings* self, uint32_t value);
		static float InternalGetThreshold(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetThreshold(ScriptScreenSpaceLensFlareSettings* self, float value);
		static uint32_t InternalGetGhostCount(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetGhostCount(ScriptScreenSpaceLensFlareSettings* self, uint32_t value);
		static float InternalGetGhostSpacing(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetGhostSpacing(ScriptScreenSpaceLensFlareSettings* self, float value);
		static float InternalGetBrightness(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetBrightness(ScriptScreenSpaceLensFlareSettings* self, float value);
		static float InternalGetFilterSize(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetFilterSize(ScriptScreenSpaceLensFlareSettings* self, float value);
		static bool InternalGetHalo(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetHalo(ScriptScreenSpaceLensFlareSettings* self, bool value);
		static float InternalGetHaloRadius(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetHaloRadius(ScriptScreenSpaceLensFlareSettings* self, float value);
		static float InternalGetHaloThickness(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetHaloThickness(ScriptScreenSpaceLensFlareSettings* self, float value);
		static float InternalGetHaloThreshold(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetHaloThreshold(ScriptScreenSpaceLensFlareSettings* self, float value);
		static float InternalGetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetHaloAspectRatio(ScriptScreenSpaceLensFlareSettings* self, float value);
		static bool InternalGetChromaticAberration(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetChromaticAberration(ScriptScreenSpaceLensFlareSettings* self, bool value);
		static float InternalGetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetChromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* self, float value);
		static bool InternalGetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* self);
		static void InternalSetBicubicUpsampling(ScriptScreenSpaceLensFlareSettings* self, bool value);
	};
}
