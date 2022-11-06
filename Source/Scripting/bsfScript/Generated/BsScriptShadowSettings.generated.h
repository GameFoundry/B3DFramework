//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ShadowSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShadowSettings : public TScriptReflectable<ScriptShadowSettings, ShadowSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ShadowSettings")

		ScriptShadowSettings(MonoObject* managedInstance, const SPtr<ShadowSettings>& value);

		static MonoObject* Create(const SPtr<ShadowSettings>& value);

	private:
		static void InternalShadowSettings(MonoObject* managedInstance);
		static float InternalGetDirectionalShadowDistance(ScriptShadowSettings* thisPtr);
		static void InternalSetDirectionalShadowDistance(ScriptShadowSettings* thisPtr, float value);
		static uint32_t InternalGetNumCascades(ScriptShadowSettings* thisPtr);
		static void InternalSetNumCascades(ScriptShadowSettings* thisPtr, uint32_t value);
		static float InternalGetCascadeDistributionExponent(ScriptShadowSettings* thisPtr);
		static void InternalSetCascadeDistributionExponent(ScriptShadowSettings* thisPtr, float value);
		static uint32_t InternalGetShadowFilteringQuality(ScriptShadowSettings* thisPtr);
		static void InternalSetShadowFilteringQuality(ScriptShadowSettings* thisPtr, uint32_t value);
	};
}
