//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ScreenSpaceReflectionsSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScreenSpaceReflectionsSettings : public TScriptReflectable<ScriptScreenSpaceReflectionsSettings, ScreenSpaceReflectionsSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ScreenSpaceReflectionsSettings")

		ScriptScreenSpaceReflectionsSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceReflectionsSettings>& value);

		static MonoObject* Create(const SPtr<ScreenSpaceReflectionsSettings>& value);

	private:
		static void InternalScreenSpaceReflectionsSettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptScreenSpaceReflectionsSettings* thisPtr);
		static void InternalSetEnabled(ScriptScreenSpaceReflectionsSettings* thisPtr, bool value);
		static uint32_t InternalGetQuality(ScriptScreenSpaceReflectionsSettings* thisPtr);
		static void InternalSetQuality(ScriptScreenSpaceReflectionsSettings* thisPtr, uint32_t value);
		static float InternalGetIntensity(ScriptScreenSpaceReflectionsSettings* thisPtr);
		static void InternalSetIntensity(ScriptScreenSpaceReflectionsSettings* thisPtr, float value);
		static float InternalGetMaxRoughness(ScriptScreenSpaceReflectionsSettings* thisPtr);
		static void InternalSetMaxRoughness(ScriptScreenSpaceReflectionsSettings* thisPtr, float value);
	};
}
