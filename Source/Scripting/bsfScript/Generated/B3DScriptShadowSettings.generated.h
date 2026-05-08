//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct ShadowSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShadowSettings : public TScriptReflectableWrapper<ShadowSettings, ScriptShadowSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShadowSettings")

		ScriptShadowSettings(const TShared<ShadowSettings>& nativeObject);
		~ScriptShadowSettings();

		static void SetupScriptBindings();

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
