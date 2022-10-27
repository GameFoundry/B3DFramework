//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs
{
	struct WhiteBalanceSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptWhiteBalanceSettings : public TScriptReflectable<ScriptWhiteBalanceSettings, WhiteBalanceSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "WhiteBalanceSettings")

		ScriptWhiteBalanceSettings(MonoObject* managedInstance, const SPtr<WhiteBalanceSettings>& value);

		static MonoObject* Create(const SPtr<WhiteBalanceSettings>& value);

	private:
		static void InternalWhiteBalanceSettings(MonoObject* managedInstance);
		static float InternalGetTemperature(ScriptWhiteBalanceSettings* thisPtr);
		static void InternalSetTemperature(ScriptWhiteBalanceSettings* thisPtr, float value);
		static float InternalGetTint(ScriptWhiteBalanceSettings* thisPtr);
		static void InternalSetTint(ScriptWhiteBalanceSettings* thisPtr, float value);
	};
} // namespace bs
