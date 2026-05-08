//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct WhiteBalanceSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptWhiteBalanceSettings : public TScriptReflectableWrapper<WhiteBalanceSettings, ScriptWhiteBalanceSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "WhiteBalanceSettings")

		ScriptWhiteBalanceSettings(const TShared<WhiteBalanceSettings>& nativeObject);
		~ScriptWhiteBalanceSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalWhiteBalanceSettings(MonoObject* scriptObject);
		static float InternalGetTemperature(ScriptWhiteBalanceSettings* self);
		static void InternalSetTemperature(ScriptWhiteBalanceSettings* self, float value);
		static float InternalGetTint(ScriptWhiteBalanceSettings* self);
		static void InternalSetTint(ScriptWhiteBalanceSettings* self, float value);
	};
}
