//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct WhiteBalanceSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptWhiteBalanceSettings : public TScriptReflectableWrapper<WhiteBalanceSettings, ScriptWhiteBalanceSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "WhiteBalanceSettings")

		ScriptWhiteBalanceSettings(const SPtr<WhiteBalanceSettings>& nativeObject);
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
