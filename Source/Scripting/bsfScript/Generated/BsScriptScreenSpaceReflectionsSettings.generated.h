//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ScreenSpaceReflectionsSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScreenSpaceReflectionsSettings : public TScriptReflectableWrapper<ScreenSpaceReflectionsSettings, ScriptScreenSpaceReflectionsSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ScreenSpaceReflectionsSettings")

		ScriptScreenSpaceReflectionsSettings(const SPtr<ScreenSpaceReflectionsSettings>& nativeObject, MonoObject* scriptObject);

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
