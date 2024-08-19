//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Image/BsColor.h"

namespace bs { struct BloomSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBloomSettings : public TScriptReflectableWrapper<BloomSettings, ScriptBloomSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "BloomSettings")

		ScriptBloomSettings(const SPtr<BloomSettings>& nativeObject, MonoObject* scriptObject);

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
