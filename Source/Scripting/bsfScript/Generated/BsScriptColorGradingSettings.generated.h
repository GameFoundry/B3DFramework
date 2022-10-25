//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct ColorGradingSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptColorGradingSettings : public TScriptReflectable<ScriptColorGradingSettings, ColorGradingSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ColorGradingSettings")

		ScriptColorGradingSettings(MonoObject* managedInstance, const SPtr<ColorGradingSettings>& value);

		static MonoObject* Create(const SPtr<ColorGradingSettings>& value);

	private:
		static void InternalGetSaturation(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void InternalSetSaturation(ScriptColorGradingSettings* thisPtr, Vector3* value);
		static void InternalGetContrast(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void InternalSetContrast(ScriptColorGradingSettings* thisPtr, Vector3* value);
		static void InternalGetGain(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void InternalSetGain(ScriptColorGradingSettings* thisPtr, Vector3* value);
		static void InternalGetOffset(ScriptColorGradingSettings* thisPtr, Vector3* __output);
		static void InternalSetOffset(ScriptColorGradingSettings* thisPtr, Vector3* value);
	};
} // namespace bs
