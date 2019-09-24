//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct WhiteBalanceSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptWhiteBalanceSettings : public TScriptReflectable<ScriptWhiteBalanceSettings, WhiteBalanceSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "WhiteBalanceSettings")

		ScriptWhiteBalanceSettings(MonoObject* managedInstance, const SPtr<WhiteBalanceSettings>& value);

		static MonoObject* create(const SPtr<WhiteBalanceSettings>& value);

	private:
		static void Internal_WhiteBalanceSettings(MonoObject* managedInstance);
		static float Internal_gettemperature(ScriptWhiteBalanceSettings* thisPtr);
		static void Internal_settemperature(ScriptWhiteBalanceSettings* thisPtr, float value);
		static float Internal_gettint(ScriptWhiteBalanceSettings* thisPtr);
		static void Internal_settint(ScriptWhiteBalanceSettings* thisPtr, float value);
	};
}
