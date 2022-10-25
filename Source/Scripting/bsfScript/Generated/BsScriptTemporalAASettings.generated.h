//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs
{
	struct TemporalAASettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTemporalAASettings : public TScriptReflectable<ScriptTemporalAASettings, TemporalAASettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "TemporalAASettings")

		ScriptTemporalAASettings(MonoObject* managedInstance, const SPtr<TemporalAASettings>& value);

		static MonoObject* Create(const SPtr<TemporalAASettings>& value);

	private:
		static void InternalTemporalAASettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptTemporalAASettings* thisPtr);
		static void InternalSetEnabled(ScriptTemporalAASettings* thisPtr, bool value);
		static uint32_t InternalGetJitteredPositionCount(ScriptTemporalAASettings* thisPtr);
		static void InternalSetJitteredPositionCount(ScriptTemporalAASettings* thisPtr, uint32_t value);
		static float InternalGetSharpness(ScriptTemporalAASettings* thisPtr);
		static void InternalSetSharpness(ScriptTemporalAASettings* thisPtr, float value);
	};
} // namespace bs
