//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	struct WhiteBalanceSettings;

	class BS_SCR_BE_EXPORT ScriptWhiteBalanceSettings : public ScriptObject<ScriptWhiteBalanceSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "WhiteBalanceSettings")

		ScriptWhiteBalanceSettings(MonoObject* managedInstance, const SPtr<WhiteBalanceSettings>& value);

		SPtr<WhiteBalanceSettings> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<WhiteBalanceSettings>& value);

	private:
		SPtr<WhiteBalanceSettings> mInternal;

		static void Internal_WhiteBalanceSettings(MonoObject* managedInstance);
		static float Internal_gettemperature(ScriptWhiteBalanceSettings* thisPtr);
		static void Internal_settemperature(ScriptWhiteBalanceSettings* thisPtr, float value);
		static float Internal_gettint(ScriptWhiteBalanceSettings* thisPtr);
		static void Internal_settint(ScriptWhiteBalanceSettings* thisPtr, float value);
	};
}
