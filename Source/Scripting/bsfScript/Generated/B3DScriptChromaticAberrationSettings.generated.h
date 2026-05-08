//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct ChromaticAberrationSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptChromaticAberrationSettings : public TScriptReflectableWrapper<ChromaticAberrationSettings, ScriptChromaticAberrationSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ChromaticAberrationSettings")

		ScriptChromaticAberrationSettings(const TShared<ChromaticAberrationSettings>& nativeObject);
		~ScriptChromaticAberrationSettings();

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
