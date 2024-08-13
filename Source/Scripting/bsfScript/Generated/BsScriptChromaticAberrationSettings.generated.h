//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ChromaticAberrationSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptChromaticAberrationSettings : public TScriptReflectable<ScriptChromaticAberrationSettings, ChromaticAberrationSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ChromaticAberrationSettings")

		ScriptChromaticAberrationSettings(MonoObject* managedInstance, const SPtr<ChromaticAberrationSettings>& value);

		static MonoObject* Create(const SPtr<ChromaticAberrationSettings>& value);

	private:
		static void InternalChromaticAberrationSettings(MonoObject* managedInstance);
		static MonoObject* InternalGetFringeTexture(ScriptChromaticAberrationSettings* self);
		static void InternalSetFringeTexture(ScriptChromaticAberrationSettings* self, MonoObject* value);
		static bool InternalGetEnabled(ScriptChromaticAberrationSettings* self);
		static void InternalSetEnabled(ScriptChromaticAberrationSettings* self, bool value);
		static ChromaticAberrationType InternalGetType(ScriptChromaticAberrationSettings* self);
		static void InternalSetType(ScriptChromaticAberrationSettings* self, ChromaticAberrationType value);
		static float InternalGetShiftAmount(ScriptChromaticAberrationSettings* self);
		static void InternalSetShiftAmount(ScriptChromaticAberrationSettings* self, float value);
	};
}
