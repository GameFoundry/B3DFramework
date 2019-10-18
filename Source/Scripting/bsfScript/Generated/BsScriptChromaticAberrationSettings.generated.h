//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ChromaticAberrationSettings; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptChromaticAberrationSettings : public TScriptReflectable<ScriptChromaticAberrationSettings, ChromaticAberrationSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ChromaticAberrationSettings")

		ScriptChromaticAberrationSettings(MonoObject* managedInstance, const SPtr<ChromaticAberrationSettings>& value);

		static MonoObject* create(const SPtr<ChromaticAberrationSettings>& value);

	private:
		static void Internal_ChromaticAberrationSettings(MonoObject* managedInstance);
		static MonoObject* Internal_getfringeTexture(ScriptChromaticAberrationSettings* thisPtr);
		static void Internal_setfringeTexture(ScriptChromaticAberrationSettings* thisPtr, MonoObject* value);
		static bool Internal_getenabled(ScriptChromaticAberrationSettings* thisPtr);
		static void Internal_setenabled(ScriptChromaticAberrationSettings* thisPtr, bool value);
		static ChromaticAberrationType Internal_gettype(ScriptChromaticAberrationSettings* thisPtr);
		static void Internal_settype(ScriptChromaticAberrationSettings* thisPtr, ChromaticAberrationType value);
		static float Internal_getshiftAmount(ScriptChromaticAberrationSettings* thisPtr);
		static void Internal_setshiftAmount(ScriptChromaticAberrationSettings* thisPtr, float value);
	};
}
