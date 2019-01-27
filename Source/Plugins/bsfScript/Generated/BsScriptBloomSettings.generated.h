//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"

namespace bs
{
	struct BloomSettings;

	class BS_SCR_BE_EXPORT ScriptBloomSettings : public ScriptObject<ScriptBloomSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "BloomSettings")

		ScriptBloomSettings(MonoObject* managedInstance, const SPtr<BloomSettings>& value);

		SPtr<BloomSettings> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<BloomSettings>& value);

	private:
		SPtr<BloomSettings> mInternal;

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
	};
}
