//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Image/BsColor.h"

namespace bs { struct BloomSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptBloomSettings : public TScriptReflectable<ScriptBloomSettings, BloomSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "BloomSettings")

		ScriptBloomSettings(MonoObject* managedInstance, const SPtr<BloomSettings>& value);

		static MonoObject* create(const SPtr<BloomSettings>& value);

	private:
		static void Internal_BloomSettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptBloomSettings* thisPtr);
		static void Internal_setenabled(ScriptBloomSettings* thisPtr, bool value);
		static uint32_t Internal_getquality(ScriptBloomSettings* thisPtr);
		static void Internal_setquality(ScriptBloomSettings* thisPtr, uint32_t value);
		static float Internal_getthreshold(ScriptBloomSettings* thisPtr);
		static void Internal_setthreshold(ScriptBloomSettings* thisPtr, float value);
		static float Internal_getintensity(ScriptBloomSettings* thisPtr);
		static void Internal_setintensity(ScriptBloomSettings* thisPtr, float value);
		static void Internal_gettint(ScriptBloomSettings* thisPtr, Color* __output);
		static void Internal_settint(ScriptBloomSettings* thisPtr, Color* value);
		static float Internal_getfilterSize(ScriptBloomSettings* thisPtr);
		static void Internal_setfilterSize(ScriptBloomSettings* thisPtr, float value);
	};
}
