//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace b3d { struct TemporalAASettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTemporalAASettings : public TScriptReflectableWrapper<TemporalAASettings, ScriptTemporalAASettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TemporalAASettings")

		ScriptTemporalAASettings(const SPtr<TemporalAASettings>& nativeObject);
		~ScriptTemporalAASettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTemporalAASettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptTemporalAASettings* self);
		static void InternalSetEnabled(ScriptTemporalAASettings* self, bool value);
		static uint32_t InternalGetJitteredPositionCount(ScriptTemporalAASettings* self);
		static void InternalSetJitteredPositionCount(ScriptTemporalAASettings* self, uint32_t value);
		static float InternalGetSharpness(ScriptTemporalAASettings* self);
		static void InternalSetSharpness(ScriptTemporalAASettings* self, float value);
	};
}
