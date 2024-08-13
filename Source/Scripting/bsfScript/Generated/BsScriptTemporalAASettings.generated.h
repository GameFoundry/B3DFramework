//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct TemporalAASettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTemporalAASettings : public TScriptReflectable<ScriptTemporalAASettings, TemporalAASettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "TemporalAASettings")

		ScriptTemporalAASettings(MonoObject* managedInstance, const SPtr<TemporalAASettings>& value);

		static MonoObject* Create(const SPtr<TemporalAASettings>& value);

	private:
		static void InternalTemporalAASettings(MonoObject* managedInstance);
		static bool InternalGetEnabled(ScriptTemporalAASettings* self);
		static void InternalSetEnabled(ScriptTemporalAASettings* self, bool value);
		static uint32_t InternalGetJitteredPositionCount(ScriptTemporalAASettings* self);
		static void InternalSetJitteredPositionCount(ScriptTemporalAASettings* self, uint32_t value);
		static float InternalGetSharpness(ScriptTemporalAASettings* self);
		static void InternalSetSharpness(ScriptTemporalAASettings* self, float value);
	};
}
