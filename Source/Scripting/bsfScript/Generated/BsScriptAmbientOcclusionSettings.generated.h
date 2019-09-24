//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct AmbientOcclusionSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAmbientOcclusionSettings : public TScriptReflectable<ScriptAmbientOcclusionSettings, AmbientOcclusionSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AmbientOcclusionSettings")

		ScriptAmbientOcclusionSettings(MonoObject* managedInstance, const SPtr<AmbientOcclusionSettings>& value);

		static MonoObject* create(const SPtr<AmbientOcclusionSettings>& value);

	private:
		static void Internal_AmbientOcclusionSettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setenabled(ScriptAmbientOcclusionSettings* thisPtr, bool value);
		static float Internal_getradius(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setradius(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float Internal_getbias(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setbias(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float Internal_getfadeDistance(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setfadeDistance(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float Internal_getfadeRange(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setfadeRange(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float Internal_getintensity(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setintensity(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static float Internal_getpower(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setpower(ScriptAmbientOcclusionSettings* thisPtr, float value);
		static uint32_t Internal_getquality(ScriptAmbientOcclusionSettings* thisPtr);
		static void Internal_setquality(ScriptAmbientOcclusionSettings* thisPtr, uint32_t value);
	};
}
