//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Utility/Image/B3DColor.h"

namespace b3d { struct BloomSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBloomSettings : public TScriptReflectableWrapper<BloomSettings, ScriptBloomSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BloomSettings")

		ScriptBloomSettings(const TShared<BloomSettings>& nativeObject);
		~ScriptBloomSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalBloomSettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptBloomSettings* self);
		static void InternalSetEnabled(ScriptBloomSettings* self, bool value);
		static uint32_t InternalGetQuality(ScriptBloomSettings* self);
		static void InternalSetQuality(ScriptBloomSettings* self, uint32_t value);
		static float InternalGetThreshold(ScriptBloomSettings* self);
		static void InternalSetThreshold(ScriptBloomSettings* self, float value);
		static float InternalGetIntensity(ScriptBloomSettings* self);
		static void InternalSetIntensity(ScriptBloomSettings* self, float value);
		static void InternalGetTint(ScriptBloomSettings* self, Color* __output);
		static void InternalSetTint(ScriptBloomSettings* self, Color* value);
		static float InternalGetFilterSize(ScriptBloomSettings* self);
		static void InternalSetFilterSize(ScriptBloomSettings* self, float value);
	};
}
