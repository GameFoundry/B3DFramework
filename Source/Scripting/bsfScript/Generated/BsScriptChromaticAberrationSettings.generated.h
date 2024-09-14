//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct ChromaticAberrationSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptChromaticAberrationSettings : public TScriptReflectableWrapper<ChromaticAberrationSettings, ScriptChromaticAberrationSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ChromaticAberrationSettings")

		ScriptChromaticAberrationSettings(const SPtr<ChromaticAberrationSettings>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalChromaticAberrationSettings(MonoObject* scriptObject);
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
