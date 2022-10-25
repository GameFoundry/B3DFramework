//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs
{
	struct ChromaticAberrationSettings;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptChromaticAberrationSettings : public TScriptReflectable<ScriptChromaticAberrationSettings, ChromaticAberrationSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ChromaticAberrationSettings")

		ScriptChromaticAberrationSettings(MonoObject* managedInstance, const SPtr<ChromaticAberrationSettings>& value);

		static MonoObject* Create(const SPtr<ChromaticAberrationSettings>& value);

	private:
		static void InternalChromaticAberrationSettings(MonoObject* managedInstance);
		static MonoObject* InternalGetFringeTexture(ScriptChromaticAberrationSettings* thisPtr);
		static void InternalSetFringeTexture(ScriptChromaticAberrationSettings* thisPtr, MonoObject* value);
		static bool InternalGetEnabled(ScriptChromaticAberrationSettings* thisPtr);
		static void InternalSetEnabled(ScriptChromaticAberrationSettings* thisPtr, bool value);
		static ChromaticAberrationType InternalGetType(ScriptChromaticAberrationSettings* thisPtr);
		static void InternalSetType(ScriptChromaticAberrationSettings* thisPtr, ChromaticAberrationType value);
		static float InternalGetShiftAmount(ScriptChromaticAberrationSettings* thisPtr);
		static void InternalSetShiftAmount(ScriptChromaticAberrationSettings* thisPtr, float value);
	};
} // namespace bs
