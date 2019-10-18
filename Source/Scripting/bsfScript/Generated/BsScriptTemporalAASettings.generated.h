//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct TemporalAASettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTemporalAASettings : public TScriptReflectable<ScriptTemporalAASettings, TemporalAASettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "TemporalAASettings")

		ScriptTemporalAASettings(MonoObject* managedInstance, const SPtr<TemporalAASettings>& value);

		static MonoObject* create(const SPtr<TemporalAASettings>& value);

	private:
		static void Internal_TemporalAASettings(MonoObject* managedInstance);
		static bool Internal_getenabled(ScriptTemporalAASettings* thisPtr);
		static void Internal_setenabled(ScriptTemporalAASettings* thisPtr, bool value);
	};
}
