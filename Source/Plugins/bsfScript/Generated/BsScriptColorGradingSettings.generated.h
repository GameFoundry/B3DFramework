//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct ColorGradingSettings;

	class BS_SCR_BE_EXPORT ScriptColorGradingSettings : public ScriptObject<ScriptColorGradingSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ColorGradingSettings")

		ScriptColorGradingSettings(MonoObject* managedInstance, const SPtr<ColorGradingSettings>& value);

		SPtr<ColorGradingSettings> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<ColorGradingSettings>& value);

	private:
		SPtr<ColorGradingSettings> mInternal;

		static void Internal_getsaturation(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void Internal_setsaturation(ScriptColorGradingSettings* thisPtr, Vector3* value);
		static void Internal_getcontrast(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void Internal_setcontrast(ScriptColorGradingSettings* thisPtr, Vector3* value);
		static void Internal_getgain(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void Internal_setgain(ScriptColorGradingSettings* thisPtr, Vector3* value);
		static void Internal_getoffset(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void Internal_setoffset(ScriptColorGradingSettings* thisPtr, Vector3* value);
	};
}
