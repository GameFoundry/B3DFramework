//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ShadowSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShadowSettings : public TScriptReflectableWrapper<ShadowSettings, ScriptShadowSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ShadowSettings")

		ScriptShadowSettings(const SPtr<ShadowSettings>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalShadowSettings(MonoObject* scriptObject);
		static float InternalGetDirectionalShadowDistance(ScriptShadowSettings* self);
		static void InternalSetDirectionalShadowDistance(ScriptShadowSettings* self, float value);
		static uint32_t InternalGetNumCascades(ScriptShadowSettings* self);
		static void InternalSetNumCascades(ScriptShadowSettings* self, uint32_t value);
		static float InternalGetCascadeDistributionExponent(ScriptShadowSettings* self);
		static void InternalSetCascadeDistributionExponent(ScriptShadowSettings* self, float value);
		static uint32_t InternalGetShadowFilteringQuality(ScriptShadowSettings* self);
		static void InternalSetShadowFilteringQuality(ScriptShadowSettings* self, uint32_t value);
	};
}
